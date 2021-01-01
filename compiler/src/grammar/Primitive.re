open Kore;

let __backslash = Uchar.of_char(C.Character.backslash);

let string =
  Character.quote
  >|= Char.context
  >>= (
    start => {
      let rec loop = f =>
        choice([
          /* end of string sequence */
          Character.quote
          >|= Char.context
          % (end_ => (String.of_uchars(f([])), Cursor.range(start, end_))),
          /* capture escaped characters */
          Character.backslash
          >> any
          >|= Char.value
          >>= (c => loop(rs => f([__backslash, c, ...rs]))),
          /* capture characters of the string */
          none_of([C.Character.quote, C.Character.eol])
          >|= Char.value
          >>= (c => loop(rs => f([c, ...rs]))),
        ]);

      loop(Functional.identity);
    }
  )
  |> M.lexeme;
