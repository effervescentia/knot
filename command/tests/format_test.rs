mod common;

use common::scratch_path;
use knot_command::format::{self, Options};
use std::fs;

fn format(name: &str, input: &str) -> String {
    let out_dir = scratch_path();
    let entry = out_dir.join(name).with_extension("kn");

    fs::write(&entry, input).expect("failed to write input file to disk");

    format::command(&Options {
        entry: entry.as_path(),
        source_dir: out_dir.as_path(),
        out_dir: out_dir.as_path(),
    });

    fs::read_to_string(entry).expect("failed to read output file from disk")
}

#[test]
fn unchanged() {
    const NAME: &str = "format_unchanged";
    const INPUT: &str = "type MyType = boolean;
enum MyEnum =
  | First(boolean, integer)
  | Second;
const MY_CONST = 100 + 20;
func my_func(first: boolean, second = 10) -> {
  let result = first || second < 5;
  result;
};
view MyView(props: string) -> <div />;
module my_module {
  const MY_STYLE = style {
    color: \"red\",
  };
}
";

    assert_eq!(format(NAME, INPUT), INPUT);
}

#[test]
fn formatted() {
    const NAME: &str = "format_module";
    const INPUT: &str = "type MyType=boolean;
enum MyEnum=First(boolean,integer)|Second;
const MY_CONST=100+20;
func my_func(first:boolean,second=10)->{let result=first| |second<5;result;};
view MyView(props:string)-><div/>;
module my_module{const MY_STYLE=style{color:\"red\"};}";
    const OUTPUT: &str = "type MyType = boolean;
enum MyEnum =
  | First(boolean, integer)
  | Second;
const MY_CONST = 100 + 20;
func my_func(first: boolean, second = 10) -> {
  let result = first || second < 5;
  result;
};
view MyView(props: string) -> <div />;
module my_module {
  const MY_STYLE = style {
    color: \"red\",
  };
}
";

    assert_eq!(format(NAME, INPUT), OUTPUT);
}
