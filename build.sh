if [[ "$1" == "final" ]]; then
	echo "Building in final mode with size optimisation."
	g++ -o conway-terminal conway-terminal.cpp -lm -lncurses -Os
else
	echo "Building in debugging mode."
	g++ -o conway-terminal conway-terminal.cpp -lm -lncurses
fi

echo "Done"
