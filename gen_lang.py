#!/usr/bin/python3

import sys, getopt, csv

import itertools

def ranges(i):
    for a, b in itertools.groupby(enumerate(i), lambda pair: pair[1] - pair[0]):
        b = list(b)
        yield b[0][1]
        yield b[-1][1]

def cencode(str):
    return str.encode("utf-8").decode("latin-1").encode("unicode-escape").decode("ascii")
def main(argv):
    language_names = []
    languages = []
    ids = []
    codepoints = []
    def addcodes(s):
        for ch in s:
            code = ord(ch)
            if code > 127 and code not in codepoints:
                codepoints.append(code)
    
    with open(argv[0], newline='') as csvfile:
        langreader = csv.reader(csvfile)
        row1 = next(langreader)
        for lang in row1[1:]:
            language_names.append(lang.split('|')[0])
            addcodes(lang.split('|')[0])
            languages.append({
                "id": lang.split('|')[1],
                "strings": []
            })
        for row in langreader:
            ids.append(row[0].replace(' ','_'))
            langidx = 0
            for translated in row[1:]:
                addcodes(translated)
                languages[langidx]["strings"].append(translated)
                langidx += 1
    codepoints.sort()
    with open(argv[1], "w") as csource:
        csource.write("//Automatically generated\n")
        csource.write("#define LANGUAGE_COUNT (%d)\n" % len(languages))
        csource.write("static const char* language_names[] = {\n")
        for lang in language_names:
            csource.write("    \"%s\",\n" % cencode(lang))
        csource.write("};\n")
        csource.write("static const char* language_codes[] = {\n")
        for lang in languages:
            csource.write("    \"%s\",\n" % cencode(lang["id"]))
        csource.write("};\n")
        for lang in languages:
            csource.write("static const char* language_%s[] = {\n" % lang["id"])
            for translated in lang["strings"]:
                csource.write("    \"%s\",\n" % cencode(translated))
            csource.write("};\n")
        csource.write("static const char** current_language = language_EN;\n")
        csource.write("static const char** languages[] = {\n")
        for lang in languages:
            csource.write("    language_%s,\n" % lang["id"])
        csource.write("};\n")
        csource.write("static ImWchar lang_ranges[] = {\n    32, 127, ")
        k = 0
        for r in ranges(codepoints):
            k += 1
            if k > 16:
                csource.write("\n    ")
                k = 0
            csource.write("0x%x, " % r)
           
        csource.write("0\n};\n")
        ids_index = 0
        for id in ids:
            csource.write("#define STR_%s (current_language[%d])\n" % (id, ids_index))
            ids_index += 1
        csource.write("#define STR_ID_COUNT (%s)\n" % ids_index)

if __name__ == "__main__":
   main(sys.argv[1:])