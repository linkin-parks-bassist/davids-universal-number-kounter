#!/usr/bin/python

import math
import re

from pathlib import Path

TAB_WIDTH = 4

lang_start = ["<?xml version=\"1.0\" encoding=\"UTF-8\"?>",
	"<language id=\"dunkasm\" name=\"DUNK Assembly\" version=\"2.0\" _section=\"Source\">",
	"\t<metadata>",
	"\t\t<property name=\"mimetypes\">text/x-dunkasm</property>",
	"\t\t<property name=\"globs\">*.dasm;*.dunkasm</property>",
	"\t\t<property name=\"line-comment-start\">%</property>",
	"\t</metadata>",
	"",
	"\t<styles>",
	"\t\t<style id=\"comment\" name=\"Comment\" map-to=\"def:comment\"/>",
	"\t\t<style id=\"keyword\" name=\"Instruction\" map-to=\"def:keyword\"/>",
	"\t\t<style id=\"register\" name=\"Register\" map-to=\"def:type\"/>",
	"\t\t<style id=\"sregister\" name=\"SRegister\" map-to=\"def:type\"/>",
	"\t\t<style id=\"number\" name=\"Number\" map-to=\"def:number\"/>",
	"\t\t<style id=\"label\" name=\"Label\" map-to=\"def:identifier\"/>",
	"\t\t<style id=\"directive\" name=\"Directive\" map-to=\"def:preprocessor\"/>",
	"\t\t<style id=\"string\" name=\"String\" map-to=\"def:string\"/>",
	"\t\t<style id=\"escaped-char\" name=\"Escaped Character\" map-to=\"def:special-char\"/>",
    "\t\t<style id=\"preprocessor\" name=\"Preprocessor\" map-to=\"def:preprocessor\"/>",
	"\t</styles>",
	"",
	"\t<definitions>",
	"\t\t<context id=\"dunkasm\">",
	"\t\t\t<include>",
	"\t\t\t\t<context ref=\"string\"/>",
	"\t\t\t\t<context ref=\"comment\"/>",
	"\t\t\t\t<context ref=\"instruction\"/>",
	"\t\t\t\t<context ref=\"register\"/>",
	"\t\t\t\t<context ref=\"number\"/>",
	"\t\t\t\t<context ref=\"label\"/>",
	"\t\t\t\t<context ref=\"escaped-char\"/>",
	"\t\t\t\t<context ref=\"label\"/>",
	"\t\t\t\t<context ref=\"preprocessor\"/>",
	"\t\t\t</include>",
	"\t\t</context>",
	"",
	"\t\t<context id=\"instruction\" style-ref=\"keyword\">"]

lang_end = ["\t\t</context>",
	"",
	"\t\t<context id=\"preprocessor\" style-ref=\"preprocessor\">",
	"\t\t\t<keyword>word</keyword>",
	"\t\t\t<keyword>include</keyword>",
	"\t\t\t<keyword>include_first</keyword>",
	"\t\t\t<keyword>alias</keyword>",
	"\t\t\t<keyword>dealias</keyword>",
	"\t\t\t<keyword>all</keyword>",
    "\t\t</context>",
	"",
	"\t\t<context id=\"register\" style-ref=\"register\">",
	"\t\t\t<match>" + r"\br[0-9a-fA-F]\b" + "</match>",
	"\t\t</context>",
	"",
	"\t\t<context id=\"sregister\" style-ref=\"sregister\">",
	"\t\t\t<match>" + r"\bsr[0-9a-fA-F]\b" + "</match>",
	"\t\t</context>",
	"",
	"\t\t<context id=\"number\" style-ref=\"number\">",
	"\t\t\t<match>" + r"\b(?:0x|0b)?[0-9A-Fa-f]+\b" + "</match>",
	"\t\t</context>",
	"",
	"\t\t<context id=\"label\" style-ref=\"label\">",
	"\t\t\t<match>" + r"\b[A-Za-z0-9_]*\b" + ":</match>",
	"\t\t</context>",
	"",
	"\t\t<context id=\"escaped-char\" style-ref=\"escaped-char\" extend-parent=\"true\">",
	"\t\t\t<match>" + r"\\[abfnrtv\'\"\?]" + "</match>",
    "\t\t</context>",
	"",
	"\t\t<context id=\"string\" style-ref=\"string\">",
	"\t\t\t<start>\\s\"</start>",
	"\t\t\t<end>\"</end>",
	"\t\t\t<include>",
	"\t\t\t\t<context ref=\"escaped-char\"/>",
	"\t\t\t\t<context id=\"string-content\" style-ref=\"string\">",
    "\t\t\t\t\t<match>[^\\\"\\n]+</match>",
    "\t\t\t\t</context>",
	"\t\t\t</include>",
	"\t\t</context>",
	"",
	"\t\t<context id=\"comment\" style-ref=\"comment\">",
	"\t\t\t<match>" + r"%[^\n]*" + "</match>",
	"\t\t</context>",
	"",
	"\t</definitions>",
	"</language>"]

def extract_instruction_names(instr_c_path: str):
	content = Path(instr_c_path).read_text()

	general_instr_pattern = re.compile(r'\b(?:NULLARY|UNARY_*|BINARY_*|TERNARY_*|SECRET_*)_INSTR\s*\(\s*"([^"]+)"\s*,\s*[^),]+')
	instructions = general_instr_pattern.findall(content)
	
	return set(instructions)

def writeout_file(instrs, output_file,
	start = [], end = [],
	line_prefix = "", line_suffix = "",
	name_prefix = "", name_suffix = "",
	print_opcodes = False,
	opcode_prefix = "", opcode_suffix = "",
	print_aligned_opcodes = False, target_col = 54,
	uppercase = False):
	lines = start
	
	for name, opcode in instrs:
		#print(f"Writeout {name}, {opcode}")
		writeout_string = line_prefix + name_prefix
		if uppercase:
			writeout_string = writeout_string + name.upper()
		else:
			writeout_string = writeout_string + name
		
		writeout_string = writeout_string + name_suffix
		
		if (print_aligned_opcodes):
			display_len = expanded_length(writeout_string)
			padding_len = target_col - display_len
			while expanded_length(writeout_string + '\t' * ((padding_len + TAB_WIDTH - 1) // TAB_WIDTH)) < target_col:
				padding_len += 1
			
			tabs_needed = (padding_len + TAB_WIDTH - 1) // TAB_WIDTH
			writeout_string = writeout_string + "\t" * tabs_needed + opcode_prefix + opcode + opcode_suffix
		elif (print_opcodes):
			writeout_string = writeout_string + opcode_prefix + opcode + opcode_suffix
		
		writeout_string = writeout_string + line_suffix
		
		lines.append(writeout_string)
	
	lines = lines + end
	
	#for line in lines:
		#print(line)
	with open(output_file, "w") as f:
		for line in lines:
			f.write(line + "\n")

def writeout_lang(instrs):
	lines = lang_start
	
	for name in instrs:
		lines.append("\t\t\t<keyword>" + name + "</keyword>")
	
	lines = lines + lang_end
	
	with open("dunkasm.lang", "w") as f:
		for line in lines:
			f.write(line + "\n")

if __name__ == '__main__':
	writeout_lang(extract_instruction_names("src/instr.c"))

