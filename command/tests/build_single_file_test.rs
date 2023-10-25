mod common;

use js::{JavaScriptGenerator, Module};
use std::collections::HashMap;

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

#[test]
fn to_javascript_esm() {
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

    let name = common::test_name(file!(), "esm");
    let result = common::build(
        &name,
        vec![("main.kn", INPUT)],
        JavaScriptGenerator::new(Module::ESM),
    );

    assert_eq!(
        result,
        HashMap::from_iter(vec![(String::from("main.js"), OUTPUT.to_string())])
    );
}

#[test]
fn to_javascript_cjs() {
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

    let name = common::test_name(file!(), "cjs");
    let result = common::build(
        &name,
        vec![("main.kn", INPUT)],
        JavaScriptGenerator::new(Module::CJS),
    );

    assert_eq!(
        result,
        HashMap::from_iter(vec![(String::from("main.js"), OUTPUT.to_string())])
    );
}
