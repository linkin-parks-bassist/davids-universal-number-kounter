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
	"\t\t<style id=\"number\" name=\"Number\" map-to=\"def:number\"/>",
	"\t\t<style id=\"label\" name=\"Label\" map-to=\"def:identifier\"/>",
	"\t\t<style id=\"directive\" name=\"Directive\" map-to=\"def:preprocessor\"/>",
	"\t\t<style id=\"string\" name=\"String\" map-to=\"def:string\"/>",
	"\t\t<style id=\"escaped-char\" name=\"Escaped Character\" map-to=\"def:special-char\"/>"
    "\t\t<style id=\"preprocessor\" name=\"Preprocessor\" map-to=\"def:preprocessor\"/>"
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
	"\t\t\t<match>" + r"\b(?:s)?r[0-9a-fA-F]\b" + "</match>",
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

def expanded_length(s: str, tab_width: int = TAB_WIDTH) -> int:
	"""Compute the visual width of a string assuming tab expansion."""
	col = 0
	for ch in s:
		if ch == '\t':
			col += tab_width - (col % tab_width)
		else:
			col += 1
	return col

def encode_param(p: str) -> str:
	suffix = ""

	if "POINTER" in p:
		suffix += "p"

	reg = ""
	if "REGISTER" in p:
		reg = "r"
		if "S_REGISTER" in p or "S_" in p:
			reg = "sr"
	suffix += reg

	if "CONSTANT" in p:
		suffix += "c"
	
	return suffix

def extract_instructions(instr_c_path: str):
	content = Path(instr_c_path).read_text()

	# Match basic instructions
	general_instr_pattern = re.compile(r'\b(?:NULLARY|UNARY_*|BINARY_*|TERNARY_*|SECRET_*)_INSTR\s*\(\s*"([^"]+)"\s*,\s*([^),]+)')
	instructions = general_instr_pattern.findall(content)

	return instructions
	
def extract_instructions_typed(instr_c_path: str):
	content = Path(instr_c_path).read_text()

	nullary_pattern = re.compile(r'(?:NULLARY|SECRET)_+INSTR\("([^"]*)",\s*([^\)]*)\)', re.MULTILINE)
	unary_pattern   = re.compile(r'UNARY___INSTR\("([^"]*)",\s*([^,]*),\s*([^,]*),\s*[^,]*,\s*\n', re.MULTILINE)
	binary_pattern  = re.compile(r'\bBINARY__INSTR\("([^"]*)",\s*([^,]*),\s*([^,]*),\s*[^,]*,\s*([^,]*),\s*[^,]*,\s*\n', re.MULTILINE)
	ternary_pattern = re.compile(r'\bTERNARY_INSTR\("([^"]*)",\s*([^,]*),\s*([^,]*),\s*[^,]*,\s*([^,]*),\s*[^,]*,\s*([^,]*),\s*[^,]*,\s*\n', re.MULTILINE)
	
	instructions_typed = list((instr.upper(), opcode.strip()) for instr, opcode in nullary_pattern.findall(content))

	
	for instr, opcode, a_type in unary_pattern.findall(content):
		suffix = f"_{encode_param(a_type)}"
		new_name = f"{instr}{suffix}"
		instructions_typed.append((new_name.upper(), opcode.strip()))
	
	for opname, opcode, a_type, b_type in binary_pattern.findall(content):
		suffix = f"_{encode_param(a_type)}_{encode_param(b_type)}"
		new_name = f"{opname}{suffix}"
		instructions_typed.append((new_name.upper(), opcode.strip()))
	
	for opname, opcode, a_type, b_type, c_type in ternary_pattern.findall(content):
		suffix = f"_{encode_param(a_type)}_{encode_param(b_type)}_{encode_param(c_type)}"
		new_name = f"{opname}{suffix}"
		instructions_typed.append((new_name.upper(), opcode.strip()))

	instructions_typed.sort(key=lambda x: int(x[1], 16))

	return instructions_typed

def extract_instruction_names(instr_c_path: str):
	instr = extract_instructions(instr_c_path)
	return set(name for name, _ in instr)
	
def writeout_header(instrs, output_file, target_col = 30):
	lines = ["#ifndef DUNKPROG_INSTR_H\n", "#define DUNKPROG_INSTR_H\n\n"]
	
	for name, opcode in instrs:
		writeout_string = "#define " + name.upper() + " "
		
		display_len = expanded_length(writeout_string)
		padding_len = target_col - display_len
		while expanded_length(writeout_string + '\t' * ((padding_len + TAB_WIDTH - 1) // TAB_WIDTH)) < target_col:
			padding_len += 1

		tabs_needed = (padding_len + TAB_WIDTH - 1) // TAB_WIDTH
		lines.append(writeout_string + "\t" * tabs_needed + opcode + "\n")
	
	lines.append("\n#endif\n")
	
	with open(output_file, "w") as f:
		for line in lines:
			f.write(line)

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
		print(f"Writeout {name}, {opcode}")
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
	
	for line in lines:
		print(line)
	with open(output_file, "w") as f:
		for line in lines:
			f.write(line + "\n")

def update_n_instr_define(filepath: str, new_value: int) -> None:
    pattern = re.compile(r'^(#define\s+N_INSTR\s+)(\d+)(\s*)$')
    with open(filepath, 'r') as f:
        lines = f.readlines()

    modified = False
    for i, line in enumerate(lines):
        match = pattern.match(line)
        if match:
            lines[i] = f"{match.group(1)}{new_value}{match.group(3)}"
            modified = True
            break

    if not modified:
        raise ValueError("No line of the form '#define N_INSTR [number]' found.")

    with open(filepath, 'w') as f:
        f.writelines(lines)

if __name__ == '__main__':
	instrs = extract_instructions("src/instr.c")
	deduped_instrs = extract_instruction_names("src/instr.c")
	typed_instrs = extract_instructions_typed("src/instr.c")
	
	print(typed_instrs)
	
	writeout_file(typed_instrs, "../dunkprog/include/opcodes.h",
		["#ifndef DUNKPROG_INSTR_H", "#define DUNKPROG_INSTR_H", ""], 
		["", "#endif"], line_prefix = "#define ", name_suffix = " ",
		print_aligned_opcodes = True, uppercase = True)
	
	writeout_file(list((name, '0x00') for name in deduped_instrs), "dunkasm.lang", lang_start, lang_end,
		line_prefix = "\t\t\t<keyword>", line_suffix = "</keyword>")
	
	writeout_file(instrs, "../../docs/instrs_itemize.tex", [r"\begin{itemize}"], [r"\end{itemize}"],
		line_prefix = "\t\\item ", name_prefix = r"\Verb|", name_suffix = "|", opcode_prefix = r": \Verb|",
		opcode_suffix = "|", print_opcodes = True)
	
	update_n_instr_define("include/instr.h", len(instrs))
