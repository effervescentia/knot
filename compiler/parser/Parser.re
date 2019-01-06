open Core;

let prog = stream => (Module.stmts << eof())(stream);

let parse = (prog, stream) =>
  switch (prog(stream)) {
  | Some((res, _)) => Some(res)
  | None => None
  };