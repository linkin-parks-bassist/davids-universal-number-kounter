all: micro_codes.pdf instruction_set.pdf overview.pdf

micro_codes.pdf :
	pdflatex micro_codes.tex

instruction_set.pdf :
	pdflatex instruction_set.tex

overview.pdf :
	pdflatex overview.tex

clean:
	rm *.aux *.out *.log *.pdf 2> /dev/null || true
