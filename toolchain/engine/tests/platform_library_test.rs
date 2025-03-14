mod common;

use assert_fs::{prelude::*, TempDir};
use knot_engine::{Analyzed, Context, Engine, Input, NoopLogger};
use kore::internal::{AmbientScope, Library, PlatformLibrary};
use std::collections::HashSet;

#[test]
#[ignore = "skip temporarily"]
fn import_between_libraries() {
    #[derive(Clone, Copy, Eq, Hash, PartialEq)]
    pub enum MockLibrary {
        Parent,
        Child,
    }

    impl PlatformLibrary for MockLibrary {
        fn text(&self) -> &str {
            match self {
                Self::Parent => {
                    "module foo {
  enum Foo {}
}"
                }
                Self::Child => {
                    "use std;
  func bar(std.foo.Foo) -> nil;"
                }
            }
        }

        fn module(&self) -> Option<&'static str> {
            None
        }
    }

    impl From<Library> for MockLibrary {
        fn from(value: Library) -> Self {
            // NOTE: this mapping is arbitrary
            match value {
                Library::Std => Self::Parent,
                Library::Ambient(_) => Self::Child,
            }
        }
    }

    impl From<MockLibrary> for Library {
        fn from(value: MockLibrary) -> Self {
            // NOTE: this mapping is arbitrary
            match value {
                MockLibrary::Parent => Self::Std,
                MockLibrary::Child => Self::Ambient(AmbientScope::Style),
            }
        }
    }

    let root_dir = TempDir::new().unwrap();
    // empty because we just want to validate the libraries
    root_dir.child("src/main.kn").write_str("").unwrap();

    let input = Input::from_entry("src/main.kn", [MockLibrary::Parent, MockLibrary::Child]);
    let engine = Engine::new(Context::new(root_dir, NoopLogger));
    let plan = Engine::plan().parse().link().analyze();

    let (_, Analyzed(analyzed)) = engine.execute(&plan, &input);

    assert_eq!(analyzed, HashSet::from([]));
}
