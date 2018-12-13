module FileStream = Knot.FileStream;

let rec lex = (chs, stream) =>
  switch (FileStream.peek(stream)) {
  | Some(('_' | 'a'..'z' | 'A'..'Z' | '0'..'9') as ch) =>
    FileStream.junk(stream);
    lex([ch, ...chs], stream);
  | Some(_)
  | None => Util.chs_to_string(chs)
  };