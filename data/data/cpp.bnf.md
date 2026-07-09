# https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/n4928.pdf


# cpp.pre
preprocessing-file:
	group?
	module-file

module-file:
	pp-global-module-fragment? pp-module group? pp-private-module-fragmentopt

pp-global-module-fragment:
	"module" ";" new-line group?

pp-private-module-fragment:
	"module" ":" "private" ";" new-line group?

group:
	group-part+

group-part:
	control-line
	if-section
	text-line
	"#" conditionally-supported-directive

control-line:
	"#" "include" pp-tokens new-line
	pp-import
	"#" "define" identifier replacement-list new-line
	"#" "define" identifier | "(" identifier-list? ")" replacement-list new-line
	"#" "define" identifier | "(" "..." ")" replacement-list new-line
	"#" "define" identifier | "(" identifier-list "," "..." ")" replacement-list new-line
	"#" "undef" identifier new-line
	"#" "line" pp-tokens new-line
	"#" "error" pp-tokens? new-line
	"#" "warning" pp-tokens? new-line
	"#" "pragma" pp-tokens? new-line
	"#" new-line

if-section:
	if-group elif-groups? else-group? endif-line

if-group:
	"#" if constant-expression new-line group?
	"#" ifdef identifier new-line group?
	"#" ifndef identifier new-line group?

elif-groups:
	elif-group
	elif-groups elif-group

elif-group:
	"#" "elif" constant-expression new-line group?
	"#" "elifdef" constant-expression new-line group?
	"#" "elifndef" constant-expression new-line group?

else-group:
	"#" "else" new-line group?

endif-line:
	"#" "endif" new-line

text-line:
	pp-tokens? new-line

conditionally-supported-directive:
	pp-tokens new-line

identifier-list:
	identifier "," identifier-list
	identifier

replacement-list:
	pp-tokensopt

pp-tokens:
	preprocessing-token+
	
new-line:
	"\n"