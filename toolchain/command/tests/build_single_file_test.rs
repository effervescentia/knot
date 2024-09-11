mod common;

use kore::{assert_eq, str};
use std::collections::HashMap;

const INPUT: &str = "type MyType = boolean;

enum MyEnum {
  | First(boolean, integer)
  | Second
}

const MY_CONST = 100 + 20;

func my_func(first: boolean, second = 10) -> {
  let result = first || second < 5;
  result;
};

view MyView { name: string } -> <></>;

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
  Second: [function Second() {}],
};
var MY_CONST = 100 + 20;
function my_func(first, second) {
  second = $knot.util.defaultParameter(second, 10);
  var result = first || second < 5;
  return result;
}
function MyView($props) {
  var name = $props.name;
  return $knot.plugin.get(\"view\", \"createFragment\", \"1.0\")();
}
var my_module = (function() {
  var MY_STYLE = $knot.plugin.get(\"style\", \"createStyle\", \"1.0\")({
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
    let result = common::build(&name, &[("main.kn", INPUT)], web::Web);

    assert_eq!(
        result.unwrap(),
        HashMap::from_iter(vec![(str!("main.js"), OUTPUT.to_owned())])
    );
}
