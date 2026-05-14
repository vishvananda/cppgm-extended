all:
	sed '/^[[:space:]]*#/d' ../pa18.gram | ./gramparse

docker:
	./generate_html.sh

clean:
	-rm -f *.output *.out
