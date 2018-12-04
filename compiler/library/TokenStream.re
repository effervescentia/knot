type token_stream = {
  file_stream: FileStream.file_stream,
  curr: ref(option((char, FileStream.file_cursor))),
};

let load = file_stream => {file_stream, curr: ref(None)};

let count = ({file_stream}) => FileStream.count(file_stream);

let next = ({file_stream} as stream) =>
  switch (FileStream.next(file_stream)) {
  | Some(res) =>
    stream.curr := Some(res);
    Some(Lexer.lex(res, file_stream));
  | None => None
  };

let reposition =
    ({file_stream}, {index, row, column}: FileStream.file_cursor) =>
  FileStream.reposition(file_stream, {index, row, column});

let peek = ({file_stream, curr} as stream) => {
  let file_cursor =
    switch (curr^) {
    | Some((_, cursor)) => cursor
    | None => {index: 0, row: 1, column: 0}
    };

  let token = next(stream);

  reposition(stream, file_cursor);
  token;
};