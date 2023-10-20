mod common;

use common::scratch_path;
use js::{JavaScriptGenerator, Module};
use knot_command::build::{self, Options};
use kore::Generator;
use lang::ast::ProgramShape;
use std::fs;

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

fn build<G>(name: &str, input: &str, generator: G) -> Option<String>
where
    G: Generator<Input = ProgramShape>,
{
    let out_dir = scratch_path();
    let entry = out_dir.join(name).with_extension("kn");

    fs::write(&entry, input).ok()?;

    build::command(&Options {
        generator,
        entry: entry.strip_prefix(&out_dir).unwrap(),
        source_dir: out_dir.as_path(),
        out_dir: out_dir.as_path(),
    });

    fs::read_to_string(entry.with_extension("js")).ok()
}

#[test]
fn to_javascript_esm() {
    const NAME: &str = "build_to_javascript_esm";

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

    let result = build(NAME, INPUT, JavaScriptGenerator::new(Module::ESM));

    assert_eq!(result.unwrap(), OUTPUT);
}

#[test]
fn to_javascript_cjs() {
    const NAME: &str = "build_to_javascript_cjs";

    const OUTPUT: &str = "var $knot = require(\"@knot/runtime\").$knot;
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
exports.MyEnum = MyEnum;
exports.MY_CONST = MY_CONST;
exports.my_func = my_func;
exports.MyView = MyView;
exports.my_module = my_module;
";

    let result = build(NAME, INPUT, JavaScriptGenerator::new(Module::CJS));

    assert_eq!(result.unwrap(), OUTPUT);
}
