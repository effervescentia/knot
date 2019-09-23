exception NotImplemented;
exception InvalidDotAccess;
exception UnanalyzedTypeReference;
exception InvalidCharacter(Uchar.t, (int, int));

let print_err = (file, error) =>
  ANSITerminal.(
    sprintf(
      [red],
      "[ERROR]: failed to parse file '%s'\n\n%s",
      file,
      String.capitalize_ascii(error),
    )
  )
  |> print_endline;
