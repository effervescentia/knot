mod common;

use common::test_path;
use js::Module;
use knot_command::{
    build::{self, Options},
    TargetFormat,
};
use std::fs;

pub fn build(name: &str, input: &str, target: TargetFormat) -> Option<String> {
    let entry = test_path(&format!(".scratch/{name}.kn"));
    let out_dir = test_path(".scratch");

    fs::write(&entry, input).ok()?;

    build::command(&Options {
        target,
        entry: entry.as_path(),
        out_dir: out_dir.as_path(),
    });

    let out_file = test_path(&format!(".scratch/{name}.js"));

    Some(fs::read_to_string(out_file).ok()?)
}

#[test]
fn to_javascript() {
    const NAME: &str = "build_to_javascript";
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
    color: \"red\"
  };
}
";
    const OUTPUT: &str = "import { $knot } from \"@knot/runtime\";
var MyEnum = {
  First: function First($param_0, $param_1) {
    return [MyEnum.First, $param_0, $param_1];
  },
  Second: function Second() {
    return [MyEnum.Second];
  },
};
var MY_CONST = 100 + 20;
function my_func(first, second) {
  second = $knot.plugin.get(\"core\", \"defaultParameter\", \"1.0\")(second, 10);
  var result = first || second < 5;
  return result;
}
function MyView(props) {
  return $knot.plugin.get(\"ksx\", \"createElement\", \"1.0\")(\"div\");
}
var my_module = (function() {
  var MY_STYLE = $knot.plugin.get(\"style\", \"create\", \"1.0\")({
    color: \"red\",
  });
  return {
    MY_STYLE: MY_STYLE,
  };
})();
export { MyEnum };
export { MY_CONST };
export { my_func };
export { MyView };
export { my_module };
";

    let result = build(NAME, INPUT, TargetFormat::JavaScript(Module::ESM));

    assert_eq!(result.unwrap(), OUTPUT);
}
