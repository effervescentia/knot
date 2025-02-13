mod common;

use assert_fs::prelude::*;
use assert_fs::TempDir;
use knot_engine::Engine;
use kore::internal::AmbientScope;
use kore::internal::Library;
use kore::internal::PlatformLibrary;

#[test]
fn use_platform_enum_in_style_expression() {
    #[derive(Clone, Copy)]
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

    let engine = Engine::new(&root_dir, false)
        .from_entry("src/main.kn")
        .include_libraries(&[MockLibrary::Std, MockLibrary::Style])
        .parse()
        .link()
        .analyze()
        .into_result();

    assert!(engine.is_ok());
}
