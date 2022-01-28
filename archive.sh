#!/bin/bash -ev

git archive --format=tar.gz -o pulsar.tar.gz --prefix=/pulsar/ HEAD .

