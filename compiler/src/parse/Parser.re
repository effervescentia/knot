/**
 Entrypoint for parsing a stream of unicode characters.
 */
let parse = (parser, input) =>
  switch (parser(input)) {
  | Some((res, _)) => Some(res)
  | None => None
  };
