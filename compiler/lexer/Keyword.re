open Knot.Token;

module FileStream = Knot.FileStream;

let rec lex = (matches, chs, stream) =>
  switch (FileStream.peek(stream)) {
  | Some(ch) =>
    let next_matches =
      List.mapi((i, (s, _)) => (s.[0], i), matches)
      |> List.filter(((char, _)) => char == ch)
      |> List.map(((_, i)) =>
           List.nth(matches, i)
           |> (((s, x)) => (String.sub(s, 1, String.length(s) - 1), x))
         );

    if (List.length(next_matches) == 1
        && fst(List.hd(next_matches))
        |> String.length == 0) {
      FileStream.junk(stream);
      switch (FileStream.peek(stream)) {
      | Some('_' | 'a'..'z' | 'A'..'Z' | '0'..'9') =>
        Identifier(Identifier.lex([ch, ...chs], stream))
      | _ => Keyword(snd(List.hd(next_matches)))
      };
    } else if (List.length(next_matches) == 0) {
      Identifier(Identifier.lex(chs, stream));
    } else {
      FileStream.junk(stream);
      lex(next_matches, [ch, ...chs], stream);
    };
  | None => Identifier(Util.chs_to_string(chs))
  };