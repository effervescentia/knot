include Knot.Globals;
include Knot.Fiber;
include Knot.Token;
include AST;

let (|=) = (x, default) => opt(default, x);

let space = one_of([Space, Tab, Newline]);
let spaces = skip_many(space);

let comma_separated = s => sep_by(s, exactly(Comma));

let lexeme = x => spaces >> x;