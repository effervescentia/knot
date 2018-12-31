open Parsing;

/* let prog = stream => (Module.body << eof())(stream); */
let prog = stream => Module.stmts(stream);

let parse = stream =>
  switch (prog(stream)) {
  | Some((res, _)) => Some(res)
  | None => None
  };