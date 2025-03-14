mod common;

use assert_fs::{prelude::*, TempDir};
use knot_engine::engine2::{Analyzed, Context, Engine, Input, NoopLogger};
use kore::internal::{AmbientScope, Library, PlatformLibrary};
use std::collections::HashSet;

#[test]
#[ignore = "skip temporarily"]
fn use_platform_enum_in_style_expression() {
    #[derive(Clone, Copy, Eq, Hash, PartialEq)]
    pub enum MockLibrary {
        Std,
        Style,
        Noop,
    }

    impl PlatformLibrary for MockLibrary {
        fn text(&self) -> &str {
            match self {
                Self::Std => "enum Color { red }",
                Self::Style => {
                    "use std;
  func color(std.Color) -> nil;"
                }
                Self::Noop => "",
            }
        }

        fn module(&self) -> Option<&'static str> {
            None
        }
    }

    impl From<Library> for MockLibrary {
        fn from(value: Library) -> Self {
            match value {
                Library::Std => Self::Std,
                Library::Ambient(AmbientScope::Style) => Self::Style,
                Library::Ambient(_) => Self::Noop,
            }
        }
    }

    impl From<MockLibrary> for Library {
        fn from(value: MockLibrary) -> Self {
            match value {
                MockLibrary::Std => Self::Std,
                MockLibrary::Style => Self::Ambient(AmbientScope::Style),
                MockLibrary::Noop => Self::Ambient(AmbientScope::Element),
            }
        }
    }

    let root_dir = TempDir::new().unwrap();
    root_dir
        .child("src/main.kn")
        .write_str(
            "use std;
const STYLE = style { color: std.Color.red };",
        )
        .unwrap();

    let input = Input::from_entry("src/main.kn", [MockLibrary::Std, MockLibrary::Style]);
    let engine = Engine::new(Context::new(root_dir, NoopLogger));
    let plan = Engine::plan().parse().link().analyze();

    let (_, Analyzed(analyzed)) = engine.execute(&plan, &input);

    assert_eq!(analyzed, HashSet::from([]));
}
