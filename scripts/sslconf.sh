#!/bin/bash

pushd $(dirname $0) > /dev/null
cd ..
root_dir=$(pwd)
popd > /dev/null

ssl_dir="$root_dir/build/ssl"
tmpl="$root_dir/scripts/openssl.conf.in"
pass=1234

mkdir -p "$ssl_dir" && cd "$ssl_dir" && rm -rf *

# root CA
cd "$ssl_dir"

m4 -Demail=root $tmpl > ca.openssl.conf
m4 -Demail=serverca $tmpl > serverCA.openssl.conf
m4 -Demail=server01 $tmpl > server01.openssl.conf
m4 -Demail=client01 $tmpl > client01.openssl.conf

# generate a certificate request
openssl req -newkey rsa:2048 -sha256 -keyout cakey.pem -out careq.pem -passout pass:$pass -config ca.openssl.conf

# print request
openssl req -in careq.pem -text -noout

# self-sign the request
openssl x509 -req -in careq.pem -sha256 -signkey cakey.pem -out cacert.pem -passin pass:$pass -extfile ca.openssl.conf -days 9999 -extensions v3_ca

# display root certificate
openssl x509 -in cacert.pem -text -noout

cat cacert.pem cakey.pem > ca.pem

## server CA signed by root
openssl req -newkey rsa:2048 -sha256 -keyout serverCAkey.pem -out serverCAreq.pem -passout pass:$pass -config serverCA.openssl.conf

openssl x509 -req -in serverCAreq.pem -sha256 -extfile serverCA.openssl.conf -extensions v3_ca -CA ca.pem -CAkey cakey.pem -CAcreateserial -out serverCAcert.pem -passin pass:$pass

cat serverCAcert.pem serverCAkey.pem cacert.pem > serverCA.pem

openssl x509 -in serverCAcert.pem -text -noout


# server01 signed by serverCA
openssl req -newkey rsa:2048 -sha256 -keyout server01key.pem -out server01req.pem -passout pass:$pass -config server01.openssl.conf

openssl x509 -req -in server01req.pem -sha256 -extfile server01.openssl.conf -extensions v3_noca -CA serverCA.pem -CAkey serverCAkey.pem -CAcreateserial -out server01cert.pem -passin pass:$pass

cat server01cert.pem server01key.pem serverCAcert.pem cacert.pem > server01.pem

openssl x509 -in server01cert.pem -text -noout

## client  signed by root
openssl req -newkey rsa:2048 -sha256 -keyout client01key.pem -out client01req.pem -passout pass:$pass -config client01.openssl.conf

openssl x509 -req -in client01req.pem -sha256 -extfile client01.openssl.conf -extensions v3_noca -CA ca.pem -CAkey cakey.pem -CAcreateserial -out client01cert.pem -passin pass:$pass

cat client01cert.pem client01key.pem cacert.pem > client01.pem

openssl x509 -in client01.pem -text -noout
