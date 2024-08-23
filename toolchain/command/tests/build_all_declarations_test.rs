#![allow(clippy::indexing_slicing)]
mod common;

use common::assert_build_single;

#[test]
fn type_alias() {
    const INPUT: &str = "type MyTypeAlias = nil;";

    const OUTPUT: &str = "import { $knot } from \"@knot/runtime\";\n";

    assert_build_single(stdext::function_name!(), INPUT, OUTPUT);
}

#[test]
fn constant() {
    const INPUT: &str = "const MY_CONSTANT: string = \"hello, world!\";";

    const OUTPUT: &str = "import { $knot } from \"@knot/runtime\";
var MY_CONSTANT = \"hello, world!\";
export { MY_CONSTANT };
";

    assert_build_single(stdext::function_name!(), INPUT, OUTPUT);
}

#[test]
fn enumerated() {
    const INPUT: &str = "
enum MyEnum =
  | Empty
  | Render(boolean, style);
";

    const OUTPUT: &str = "import { $knot } from \"@knot/runtime\";
var MyEnum = {
  Empty: function Empty() {
    return [MyEnum.Empty];
  },
  Render: function Render($param_0, $param_1) {
    return [MyEnum.Render, $param_0, $param_1];
  },
};
export { MyEnum };
";

    assert_build_single(stdext::function_name!(), INPUT, OUTPUT);
}

#[test]
fn function() {
    const INPUT: &str = "func my_function(first: float, second: integer, third = true): boolean -> first > second || third;";

    const OUTPUT: &str = "import { $knot } from \"@knot/runtime\";
function my_function(first, second, third) {
  third = $knot.plugin.get(\"core\", \"defaultParameter\", \"1.0\")(third, true);
  return first > second || third;
}
export { my_function };
";

    assert_build_single(stdext::function_name!(), INPUT, OUTPUT);
}

#[test]
fn view() {
    const INPUT: &str = "
view Greet(name: string) -> <>Welcome {name}!</>;

const GREETING = <Greet name=\"Alex\" />;
";

    const OUTPUT: &str = "import { $knot } from \"@knot/runtime\";
function Greet($props) {
  var name = $props.name;
  return $knot.plugin.get(\"view\", \"createFragment\", \"1.0\")(\"Welcome \", name, \"!\");
}
var GREETING = $knot.plugin.get(\"view\", \"createElement\", \"1.0\")(Greet, {
  name: \"Alex\",
});
export { Greet };
export { GREETING };
";

    assert_build_single(stdext::function_name!(), INPUT, OUTPUT);
}

#[test]
fn module() {
    const INPUT: &str = "
module my_module {
  const FOO = 123;
}

const BAR = my_module.FOO;
";

    const OUTPUT: &str = "import { $knot } from \"@knot/runtime\";
var my_module = (function() {
  var FOO = 123;
  return {
    FOO: FOO,
  };
})();
var BAR = my_module.FOO;
export { my_module };
export { BAR };
";

    assert_build_single(stdext::function_name!(), INPUT, OUTPUT);
}
