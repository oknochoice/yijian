#!/bin/bash
protoc --proto_path=./ --cpp_out=./ ./chat_message.proto
protoc --swift_out=. ./chat_message.proto
