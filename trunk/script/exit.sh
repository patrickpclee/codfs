#!/bin/sh

kill -INT `ps ux | awk '/CLIENT/ && !/awk/ {print $2}'`
echo "CLIENT terminated."
