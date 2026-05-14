all:
	sed '/^[[:space:]]*#/d' ../pa26.gram | ./gramparse

docker:
	./generate_html.sh

clean:
	-rm -f *.output *.out
