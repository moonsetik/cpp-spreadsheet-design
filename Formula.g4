grammar Formula;

main
    : expr EOF
    ;

expr
    : '(' expr ')'            # Parens
    | (ADD | SUB) expr        # UnaryOp
    | expr (MUL | DIV) expr   # BinaryOp
    | expr (ADD | SUB) expr   # BinaryOp
    | NUMBER                  # Literal
    | CELL                    # CellRef
    ;

fragment INT: [-+]? UINT ;
fragment UINT: [0-9]+ ;
fragment EXPONENT: [eE] INT;

NUMBER
    : UINT EXPONENT?
    | UINT? '.' UINT EXPONENT?
    ;

CELL
    : [A-Za-z]+ [0-9]+
    ;

ADD: '+' ;
SUB: '-' ;
MUL: '*' ;
DIV: '/' ;
WS: [ \t\n\r]+ -> skip ;