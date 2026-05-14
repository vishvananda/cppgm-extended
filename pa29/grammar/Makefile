all:
	sed '/^[[:space:]]*#/d' ../pa29.gram | ./gramparse

docker:
	./generate_html.sh

clean:
	-rm -f *.output *.out
