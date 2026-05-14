all:
	sed '/^[[:space:]]*#/d' ../pa19.gram | ./gramparse

docker:
	./generate_html.sh

clean:
	-rm -f *.output *.out
