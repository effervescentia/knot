open Knot.Token;

module FileStream = Knot.FileStream;

exception UnclosedString;

let rec lex = (chs, stream) =>
  switch (FileStream.next(stream)) {
  | Some((ch, _)) =>
    switch (ch) {
    | '"' =>
      List.rev(chs)
      |> List.fold_left((acc, c) => acc ++ String.make(1, c), "")
    | _ => lex([ch, ...chs], stream)
    }
  | None => raise(UnclosedString)
  };