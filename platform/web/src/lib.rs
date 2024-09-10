use kore::internal;

pub struct Web;

impl internal::Platform for Web {
    type Library = Library;
    type Generator = js::Generator<Library>;
    type Program = lang::ast::shape::Program;

    fn libraries() -> Vec<Self::Library> {
        vec![Library::Std, Library::Html, Library::Css]
    }

    fn generator() -> Self::Generator {
        js::Generator::new(js::Module::ESM)
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum Library {
    Std,
    Html,
    Css,
}

impl Library {
    pub const STD: &'static str = include_str!("../libs/lib_std.kd");
    pub const HTML: &'static str = include_str!("../libs/lib_html.kd");
    pub const CSS: &'static str = include_str!("../libs/lib_css.kd");
}

impl internal::PlatformLibrary for Library {
    fn text(&self) -> &str {
        match self {
            Self::Std => Self::STD,
            Self::Html => Self::HTML,
            Self::Css => Self::CSS,
        }
    }

    fn module(&self) -> Option<&'static str> {
        match self {
            Self::Std => Some("std/browser"),

            _ => None,
        }
    }
}

impl From<Library> for internal::Library {
    fn from(value: Library) -> Self {
        match value {
            Library::Std => Self::Std,
            Library::Html => Self::Ambient(internal::AmbientScope::Element),
            Library::Css => Self::Ambient(internal::AmbientScope::Style),
        }
    }
}

impl From<internal::Library> for Library {
    fn from(value: internal::Library) -> Self {
        match value {
            internal::Library::Std => Self::Std,
            internal::Library::Ambient(internal::AmbientScope::Element) => Self::Html,
            internal::Library::Ambient(internal::AmbientScope::Style) => Self::Css,
        }
    }
}
