open Kore;

module CodeFrame = File.CodeFrame;

let __file_contents = Util.read_file_to_string(multiline_path);

let suite =
  "File.CodeFrame"
  >::: [
    "print"
    >: (
      () =>
        [
          (
            " 1 │ Ben Barker
 2 │ Alex Ainsley
 3 │ Vince Varden
   │ ^^^^^^^^^^^^
 4 │ Willow Eve Witts
 5 │ Carson Coppota
",
            CodeFrame.print(
              __file_contents,
              Cursor.range((3, 1), (3, 12)),
            ),
          ),
          (
            " 2 │ Alex Ainsley
 3 │ Vince Varden
 4 │ Willow Eve Witts
   │        ^^^
 5 │ Carson Coppota
 6 │ \n",
            CodeFrame.print(
              __file_contents,
              Cursor.range((4, 8), (4, 10)),
            ),
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ];
