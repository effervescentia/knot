use super::walk::{self, WalkEach};
use crate::Range;
use std::fmt::Debug;

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum ImportSource {
    Root,
    Local,
    Named(String),
    Scoped { scope: String, name: String },
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct Import {
    pub source: ImportSource,
    pub path: Vec<String>,
    pub alias: Option<String>,
}

impl Import {
    pub fn new(source: ImportSource, path: Vec<String>, alias: Option<String>) -> Self {
        Self {
            source,
            path,
            alias,
        }
    }
}

impl<Visitor, Meta> walk::Walk<Visitor, (Range, Meta)> for walk::Span<Import, Meta>
where
    Visitor: walk::Visit<(Range, Meta)>,
{
    type Output = Visitor::Import;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (
            super::Import {
                source,
                path,
                alias,
            },
            ctx,
        ) = self;

        v.import(
            super::Import {
                source,
                path,
                alias,
            },
            ctx,
        )
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct Module<Import, Declaration> {
    pub imports: Vec<Import>,
    pub declarations: Vec<Declaration>,
}

impl<Import, Declaration> Module<Import, Declaration> {
    pub fn new(imports: Vec<Import>, declarations: Vec<Declaration>) -> Self {
        Self {
            imports,
            declarations,
        }
    }
}

impl<Visitor, Meta, Import, Declaration> walk::Walk<Visitor, (Range, Meta)>
    for walk::Span<Module<Import, Declaration>, Meta>
where
    Visitor: walk::Visit<(Range, Meta)>,
    Import: walk::Walk<Visitor, (Range, Meta), Output = Visitor::Import>,
    Declaration: walk::Walk<Visitor, (Range, Meta), Output = Visitor::Declaration>,
{
    type Output = Visitor::Module;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (
            super::Module {
                imports,
                declarations,
            },
            ctx,
        ) = self;
        let ((imports, declarations), v) = (imports, declarations).walk_each(v);

        v.module(
            super::Module {
                imports,
                declarations,
            },
            ctx,
        )
    }
}
