open Core;

module Op = Operator;

let variable = M.identifier ==> (s => Variable(s));
let dot_access = chainl1(variable, Op.dot);

let expr = dot_access <|> variable;
