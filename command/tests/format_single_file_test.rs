mod common;

#[test]
fn unchanged() {
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

    let name = common::test_name(file!(), "unchanged");
    assert_eq!(common::format(&name, INPUT).unwrap(), INPUT);
}

#[test]
fn formatted() {
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

    let name = common::test_name(file!(), "formatted");
    assert_eq!(common::format(&name, INPUT).unwrap(), OUTPUT);
}
