
# custom wrappers
kw-template:  
	"template"
	
namespace-accessor:  
	"::"

elipsis:  
	"..."

comma:  
	","

# class
class-name:  
	identifier
	simple-template-id

# dcl.attr.grammar  
attribute-specifier-seq:  
	attribute-specifier  
	attribute-specifier-seq attribute-specifier

attribute-specifier:  
	"[" "[" attribute-list "]" "]"  
	alignment-specifier
	
alignment-specifier:  
	"alignas" "(" type-id elipsis? ")"
	"alignas" "(" assignment-expression elipsis? ")"
	
attribute-list:  
	attribute?
	attribute-list "," attribute?
	attribute "..."
	attribute-list "," attribute "..."

attribute:  
	attribute-token attribute-argument-clause?
	
attribute-token:  
	identifier
	attribute-scoped-token
	
attribute-scoped-token:  
	attribute-namespace "::" identifier

attribute-namespace:  
	identifier
	
attribute-argument-clause:  
	"(" balanced-token-seq ")"

balanced-token-seq:  
	balanced-token
	balanced-token-seq balanced-token
	
balanced-token:  
	"(" balanced-token-seq ")"
	"[" balanced-token-seq "]"
	"{" balanced-token-seq "}"
	
balanced-token-token:  
	identifier
	keyword
	literal
	operator-token
	balanced-token-punctuator
	
	
balanced-token-punctuator:  
	"%:"
	"%:%:"
	"#"
	"##"
	";"
	":"
	"..."
	"new"
	"delete"
	"?"
	"::"
	"."
	".*"
	"+"
	"-"
	"*"
	"/"
	"%"
	"^"
	"&"
	"|"
	"~"
	"!"
	"="
	"<"
	">"
	"+="
	"-="
	"*="
	"/="
	"%="
	"^="
	"&="
	"|="
	"<<"
	">>"
	"<<="
	">>="
	"=="
	"!="
	"<="
	">="
	"&&"
	"||"
	"++"
	"--"
	","
	"->*"
	"->"
	"and"
	"and_eq"
	"bitand"
	"bitor"
	"compl"
	"not"
	"not_eq"
	"or"
	"or_eq"
	"xor"
	"xor_eq"

# dcl.init
initializer-clause:  
	assignment-expression
	braced-init-list

initializer-list:  
	initializer-clause elipsis?
	initializer-list "," initializer-clause elipsis?

braced-init-list:  
	"{" initializer-list comma? "}"

# dcl.decl
declarator:
	ptr-declarator
	noptr-declarator parameters-and-qualifiers trailing-return-type

ptr-declarator:
	noptr-declarator
	ptr-declarator ptr-declarator

noptr-declarator:
	declarator-id attribute-specifier-seq?
	noptr-declarator parameters-and-qualifiers
	noptr-declarator "[" constant-expression? "]" attribute-specifier-seq?
	"(" ptr-declarator ")"

parameters-and-qualifiers:  
	"(" parameter-declaration-clause ")" attribute-specifier-seq? cv-qualifier-seq? ref-qualifier?exception-specification?

trailing-return-type:  
	"->" trailing-type-specifier-seq abstract-declarator?

cv-qualifier-seq:  
	cv-qualifier+

cv-qualifier:  
	const
	volatile

ref-qualifier:  
	"&"
	"&&"

# dcl.fct
parameter-declaration-clause:  
	parameter-declaration-list? elipsis?
	parameter-declaration-list "," "..."

parameter-declaration-list:  
	parameter-declaration
	parameter-declaration-list "," parameter-declaration
	
parameter-declaration:  
	attribute-specifier-seq? decl-specifier-seq declarator
	attribute-specifier-seq? decl-specifier-seq declarator "=" initializer-clause
	attribute-specifier-seq? decl-specifier-seq abstract-declarator?
	attribute-specifier-seq? decl-specifier-seq abstract-declarator? "=" initializer-clause

# dcl.spec
decl-specifier:
	storage-class-specifier
	type-specifier
	function-specifier
	"friend"
	"typedef"
	"constexpr"

decl-specifier-seq:
	decl-specifier+ attribute-specifier-seq?

# dcl.type
trailing-type-specifier-seq:  
	trailing-type-specifier attribute-specifier-seq?
	trailing-type-specifier trailing-type-specifier-seq

# decl.name
type-id:  
	type-specifier-seq abstract-declarator?
	
abstract-declarator:  
	ptr-abstract-declarator
	noptr-abstract-declarator? parameters-and-qualifiers trailing-return-type
	"..."

ptr-abstract-declarator:  
	noptr-abstract-declarator
	ptr-operator ptr-abstract-declarator?

noptr-abstract-declarator:  
	noptr-abstract-declarator? parameters-and-qualifiers
	noptr-abstract-declarator? "[" constant-expression "]" attribute-specifier-seq?
	"(" ptr-abstract-declarator ")"

# dct.type.simple
simple-type-specifier:
	namespace-accessor? nested-name-specifier? type-name
	namespace-accessor? nested-name-specifier "template" simple-template-id
	"char"
	"char16_t"
	"char32_t"
	"wchar_t"
	"bool"
	"short"
	"int"
	"long"
	"signed"
	"unsigned"
	"float"
	"double"
	"void"
	"auto"
	decltype-specifier

type-name:
	class-name
	enum-name
	typedef-name
	simple-template-id

decltype-specifier:  
	"decltype" "(" expression ")"

# temp.names
simple-template-id:  
	template-name "<" template-argument-list? ">"

template-id:  
	simple-template-id
	operator-function-id "<" template-argument-list? ">"
	literal-operator-id "<" template-argument-list ">"

template-name:  
	identifier
	
template-argument-list:  
	template-argument elipsis?
	template-argument-list "," template-argument elipsis?
	
template-argument:  
	constant-expression
	type-id
	id-expression

# except.spec
exception-specification:  
	dynamic-esception-specification
	noexcept-specification
	
dynamic-esception-specification:  
	"throw" "(" type-id-list? ")"

type-id-list:  
	type-id elipsis?
	type-id-list "," type-id elipsis?

noexcept-specification:  
	"noexcept" "(" constant-expression ")"
	"noexcept"

# expr.prim.general
primary-expression:  
	literal
	"this"
	"(" expression ")"
	id-expression
	lambda-expression

id-expression:  
	unqualified-id
	qualified-id

unqualified-id:  
	identifier
	operator-function-id
	conversion-function-id
	literal-operator-id
	"~" class-name
	"~" decltype-specifier
	template-id
	
qualified-id:  
	namespace-accessor? nested-name-specifier kw-template? unqualified-id
	"::" identifier
	"::" operator-function-id
	"::" literal-operator-id
	"::" template-id
	
nested-name-specifier:  
	type-name "::"
	namespace-name "::"
	decltype-specifier "::"
	nested-name-specifier identifier "::"
	nested-name-specifier kw-template? simple-template-id "::"

# expr.post
postfix-expression:  
	primary-expression
	postfix-expression "[" expression "]"
	postfix-expression "[" braced-init-list? "]"
	postfix-expression "(" expression-list? ")"
	simple-type-specifier "(" expression-list? ")"
	typename-specifier "(" expression-list? ")"
	simple-type-specifier braced-init-list
	typename-specifier braced-init-list
	postfix-expression "." kw-template? id-expression
	postfix-expression "->" kw-template? id-expression
	postfix-expression "++"
	postfix-expression "--"
	"dynamic_cast" "<" type-id ">" "(" expression ")"
	"static_cast" "<" type-id ">" "(" expression ")"
	"reinterpret_cast" "<" type-id ">" "(" expression ")"
	"const_cast" "<" type-id ">" "(" expression ")"
	"typeid" "(" expression ")"
	"typeid" "(" type-id ")"

expression-list:  
	initializer-list
	
pseudo-destructor-name:  
	namespace-accessor? nested-name-specifier? type-name "::" "~" type-name
	namespace-accessor? nested-name-specifier "template" simple-template-id "::" "~" type-name
	namespace-accessor? nested-name-specifier? "~" type-name
	"~" decltype-specifier

# expr.unary
unary-expression:  
	postfix-expression
	"++" cast-expression
	"--" cast-expression
	unary-operator cast-expression
	"sizeof" unary-expression
	"sizeof" "(" type-id ")"
	"sizeof" "..." "(" identifier ")"
	"alignof" "(" type-id ")"
	noexcept-expression
	new-expression
	delete-expression
	
unary-operator:  
	"*"
	"&"
	"+"
	"-"
	"!"
	"~"

# expr.cast
cast-expression:  
	unary-expression
	"(" type-id ")" cast-expression

# expr.mptr.oper
pm-expression:  
	cast-expression
	pm-expression ".*" cast-expression
	pm-expression "->*" cast-expression

# expr.mul
multiplicative-expression:  
	pm-expression
	multiplicative-expression "*" pm-expression
	multiplicative-expression "/" pm-expression
	multiplicative-expression "%" pm-expression

# expr.add
addative-expression:  
	multiplicative-expression
	addative-expression "+" multiplicative-expression
	addative-expression "-" multiplicative-expression

# expr.shift
shift-expression:  
	addative-expression
	shift-expression "<<" addative-expression
	shift-expression ">>" addative-expression

# expr.rel
relational-expression:  
	shift-expression
	relational-expression "<" shift-expression
	relational-expression ">" shift-expression
	relational-expression "<=" shift-expression
	relational-expression ">=" shift-expression

# expr.eq
equality-expression:  
	relational-expression
	equality-expression "==" relational-expression
	equality-expression "!=" relational-expression

# expr.bit.and
and-expression:  
	equality-expression
	and-expression "&" equality-expression

# expr.xor
exclusive-or-expression:  
	and-expression
	exclusive-or-expression "^" and-expression

# expr.or
inclusive-or-expression:  
	exclusive-or-expression
	inclusive-or-expression "|" exclusive-or-expression

# expr.log.or
logical-and-expression:  
	inclusive-or-expression
	logical-and-expression "&&" inclusive-or-expression

# expr.log.or
logical-or-expression:  
	logical-and-expression
	logical-or-expression "||" logical-and-expression

# expr.cond
conditional-expression:  
	logical-or-expression
	logical-or-expression "?" expression ":" assignment-expression

# expr.ass
assignment-expression:  
	conditional-expression
	logical-or-expression assignment-operator initializer-clause
	throw-expression
	
assignment-operator:  
	"="
	"*="
	"/="
	"%="
	"+="
	"-="
	">>="
	"<<="
	"&="
	"^="
	"|="

# expr.comma
expression:  
	assignment-expression
	expression "," assignment-expression

# expr.const
constant-expression:  
	conditional-expression

# over.literal
literal-operator-id:  
	"operator" quote | quote identifier

# over.oper
operator-function-id:  
	"operator" overloadable-operator
	"operator" overloadable-operator "<" template-argument-list? ">"

overloadable-operator:  
	"new"
	"delete"
	"new" "[" "]"
	"delete" "[" "]"
	"+"
	"-"
	"*"
	"/"
	"%"
	"^"
	"&"
	"|"
	"~"
	"!"
	"="
	"<"
	">"
	"+="
	"-="
	"*="
	"/="
	"%="
	"^="
	"&="
	"|="
	"<<"
	">>"
	"<<="
	">>="
	"=="
	"!="
	"<="
	">="
	"&&"
	"||"
	"++"
	"--"
	","
	"->*"
	"->"
	"()"
	"[]"

# cpp
preprocessing-file:  
 	group?

group:  
 	group-part+

group-part:  
 	if-section
 	control-line
 	text-line
 	"#" non-directive

if-section:  
 	if-group elif-groups? else-group? endif-line

if-group:  
 	"#" "if" constant-expression new-line group?
 	"#" "ifdef" identifier new-line group?
 	"#" "ifndef" identifier new-line group?

elif-groups:  
 	elif-group
 	elif-groups elif-group

elif-group:  
 	"#" "elif" constant-expression new-line group?

else-group:  
 	"#" "else" new-line group?

endif-line:  
 	"#" "endif" new-line

control-line:  
 	"#" "include" pp-tokens new-line
 	"#" "define" identifier replacement-list new-line
 	"#" "define" identifier | "(" identifier-list? ")" replacement-list new-line
 	"#" "define" identifier | "(" identifier-list "," "..." ")" replacement-list new-line
 	"#" "undef" identifier new-line
 	"#" "line" pp-tokens new-line
 	"#" "error" pp-tokens? new-line
 	"#" "pragma" pp-tokens? new-line
 	"#" new-line

text-line:  
 	pp-tokens? new-line

non-directive:  
 	pp-tokens new-line

identifier-list:  
 	identifier
 	identifier-list "," identifier

replacement-list:  
 	pp-tokens?

pp-tokens:  
 	preprocessing-token+

new-line:  
	"\n"
	"\r\n"

# lex.name
identifier:  
	identifier-nondigit
	identifier identifier-nondigit
	identifier digit

identifier-nondigit:  
	nondigit
	universal-character-name
	
non-digit:  
	"a"
	"b"
	"c"
	"d"
	"e"
	"f"
	"g"
	"h"
	"i"
	"j"
	"k"
	"l"
	"m"
	"n"
	"o"
	"p"
	"q"
	"r"
	"s"
	"t"
	"u"
	"v"
	"w"
	"x"
	"y"
	"z"
	"A"
	"B"
	"C"
	"D"
	"E"
	"F"
	"G"
	"H"
	"I"
	"J"
	"K"
	"L"
	"M"
	"N"
	"O"
	"P"
	"Q"
	"R"
	"S"
	"T"
	"U"
	"V"
	"W"
	"Z"
	"Y"
	"Z"
	"_"
	
digit:  
	"0"
	"1"
	"2"
	"3"
	"4"
	"5"
	"6"
	"7"
	"8"
	"9"