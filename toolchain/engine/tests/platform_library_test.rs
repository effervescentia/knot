mod common;

use assert_fs::prelude::*;
use assert_fs::TempDir;
use knot_engine::Engine;
use kore::internal::AmbientScope;
use kore::internal::Library;
use kore::internal::PlatformLibrary;

#[test]
fn import_between_libraries() {
    #[derive(Clone, Copy)]
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

    let engine = Engine::new(&root_dir, false)
        .from_entry("src/main.kn")
        .include_libraries(&[MockLibrary::Parent, MockLibrary::Child])
        .parse()
        .link()
        .analyze()
        .into_result();

    assert!(engine.is_ok());
}
