#!/bin/bash

pdflatex -output-directory=build main.tex
bibtex build/main.aux
pdflatex -output-directory=build main.tex
pdflatex -output-directory=build main.tex