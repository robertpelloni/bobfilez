#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CP="$SCRIPT_DIR/build/classes"
CP="$CP:$SCRIPT_DIR/picocli-4.7.5.jar"
CP="$CP:$SCRIPT_DIR/sqlite-jdbc-3.45.1.0.jar"
CP="$CP:$SCRIPT_DIR/slf4j-api-2.0.9.jar"
CP="$CP:$SCRIPT_DIR/slf4j-simple-2.0.9.jar"

java -Xmx256m -Xms64m -cp "$CP" com.filez.cli.FilezApp "$@"
