FROM ubuntu:bionic

RUN apt-get update && \
	apt-get install -y \
  build-essential

COPY . /sistemi-operativi/
WORKDIR /sistemi-operativi/
