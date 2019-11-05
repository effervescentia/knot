include Knot.Globals;
include Knot.Token;

let rec (===>) = Matcher.((s, t) => token(s, _ => result(t)));
