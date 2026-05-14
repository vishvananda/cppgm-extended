all:
	sed '/^[[:space:]]*#/d' ../pa16.gram | ./gramparse

docker:
	./generate_html.sh

clean:
	-rm -f *.output *.out
