module FileStream = Knot.FileStream;

let rec lex = (chs, stream) =>
  switch (FileStream.peek(stream)) {
  | Some(ch) =>
    switch (ch) {
    | '0'..'9' =>
      FileStream.junk(stream);
      lex([ch, ...chs], stream);
    | _ => Util.chs_to_number(chs)
    }
  | None => Util.chs_to_number(chs)
  };