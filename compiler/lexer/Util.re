module FileStream = Knot.FileStream;

let chs_to_string = chs =>
  List.rev(chs) |> List.fold_left((acc, c) => acc ++ String.make(1, c), "");
let chs_to_number = chs => chs_to_string(chs) |> int_of_string;

let rec next_non_space = stream =>
  switch (FileStream.peek(stream)) {
  | Some(' ' | '\t' | '\n') =>
    FileStream.junk(stream);
    next_non_space(stream);
  | Some(tkn) => Some(tkn)
  | None => None
  };

let peek_next_non_space = (stream, cursor) => {
  let char = next_non_space(stream);
  FileStream.reposition(stream, cursor);
  char;
};

let junk_non_space = stream => {
  next_non_space(stream) |> ignore;
  FileStream.junk(stream);
};