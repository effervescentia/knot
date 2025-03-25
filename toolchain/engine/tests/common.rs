#![allow(dead_code)]
use assert_fs::prelude::*;
use assert_fs::TempDir;
use knot_engine::{Analyzed, Context, Engine, Input, NoopLogger};
use kore::internal::{Library, PlatformLibrary};

pub fn assert_valid(source: &str) {
    let root_dir = TempDir::new().unwrap();
    root_dir.child("src/main.kn").write_str(source).unwrap();

    let input = Input::from_entry("src/main.kn", [MockLibrary]);
    let engine = Engine::new(Context::new(root_dir, NoopLogger));
    let plan = Engine::plan().parse_and_traverse().link().analyze();

    let (_, Analyzed(analyzed)) = engine.execute(&plan, &input);

    assert!(!analyzed.is_empty());
}

#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub struct MockLibrary;

impl PlatformLibrary for MockLibrary {
    fn text(&self) -> &str {
        ""
    }

    fn module(&self) -> Option<&'static str> {
        None
    }
}

impl From<Library> for MockLibrary {
    fn from(_: Library) -> Self {
        Self
    }
}

impl From<MockLibrary> for Library {
    fn from(_: MockLibrary) -> Self {
        // NOTE: this mapping is arbitrary
        Self::Std
    }
}
