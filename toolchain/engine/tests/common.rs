#![allow(dead_code)]
use assert_fs::prelude::*;
use assert_fs::TempDir;
use knot_engine::Engine;
use kore::internal::{Library, PlatformLibrary};

pub fn assert_valid(source: &str) -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir.child("src/main.kn").write_str(source)?;

    let engine = Engine::new(&root_dir, false)
        .from_entry("src/main.kn")
        .include_libraries(&[MockLibrary])
        .parse()
        .link()
        .analyze()
        .into_result();

    assert!(engine.is_ok());

    Ok(())
}

#[derive(Clone, Copy)]
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
