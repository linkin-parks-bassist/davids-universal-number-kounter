all: micro_codes.pdf instruction_set.pdf overview.pdf

micro_codes.pdf : micro_codes.tex
	pdflatex micro_codes.tex

instruction_set.pdf : instruction_set.tex
	pdflatex instruction_set.tex

overview.pdf : overview.tex
	pdflatex overview.tex

clean:
	rm *.aux *.out *.log *.pdf 2> /dev/null || true
