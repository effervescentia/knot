mod common;

use common::test_path;
use knot_command::format::{self, Options};
use std::fs;

pub fn format(name: &str, input: &str) -> Option<String> {
    let entry = test_path(&format!(".scratch/{name}.kn"));
    let out_dir = test_path(".scratch");

    fs::write(&entry, input).ok()?;

    format::command(&Options {
        entry: entry.as_path(),
        out_dir: out_dir.as_path(),
    });

    Some(fs::read_to_string(entry).ok()?)
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

    let result = format(NAME, INPUT);

    assert_eq!(result.unwrap(), INPUT);
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

    let result = format(NAME, INPUT);

    assert_eq!(result.unwrap(), OUTPUT);
}
