all:
	sed '/^[[:space:]]*#/d' ../pa30.gram | ./gramparse

docker:
	./generate_html.sh

clean:
	-rm -f *.output *.out
