use super::{Ast, Module};
use crate::{report, Library, Link};
use bimap::BiMap;
use kore::Incrementor;
use lang::{
    walk::{CommonVisitor, ProgramVisitor, TypingsVisitor, Walk},
    CanonicalId, NamespaceId, NodeId, Range,
};
use std::{collections::HashMap, marker::PhantomData};

pub type ModuleIterator<'a, T> =
    Box<dyn std::iter::Iterator<Item = (&'a Link, &'a super::Module<T>)> + 'a>;

#[derive(Clone)]
pub struct Base<T> {
    modules: HashMap<Link, super::Module<T>>,
    lookup: BiMap<Link, NamespaceId>,
    ambient: analyze::AmbientMap,
    verbose: bool,
}

impl<T> Base<T> {
    pub const fn ambient(&self) -> &analyze::AmbientMap {
        &self.ambient
    }

    pub fn new(verbose: bool) -> Self {
        Self {
            modules: Default::default(),
            lookup: Default::default(),
            ambient: Default::default(),
            verbose,
        }
    }

    pub fn has_by_link(&self, link: &Link) -> bool {
        self.modules.contains_key(link)
    }

    pub fn get_id_by_link(&self, link: &Link) -> Option<&NamespaceId> {
        self.lookup.get_by_left(link)
    }

    pub fn get_link_by_id(&self, id: &NamespaceId) -> Option<&Link> {
        self.lookup.get_by_right(id)
    }

    pub fn get_module_by_link(&self, link: &Link) -> Option<&Module<T>> {
        self.modules.get(link)
    }

    pub fn get_link_and_module_by_id(&self, id: &NamespaceId) -> Option<(&Link, &Module<T>)> {
        let link = self.get_link_by_id(id)?;

        Some((link, self.modules.get(link)?))
    }

    pub fn modules(&self) -> ModuleIterator<T> {
        Box::new(self.modules.iter())
    }

    pub fn internal_modules(&self) -> ModuleIterator<T> {
        Box::new(
            self.modules
                .iter()
                .filter_map(|(link, module)| link.is_internal().then_some((link, module))),
        )
    }

    pub fn register_library(&mut self, library: Library, link: Link, module: Module<T>) {
        if let Some(scope) = library.to_ambient_scope() {
            self.ambient.insert(scope, module.id);
        }

        self.modules.insert(link, module);
    }

    pub fn register_module(&mut self, link: Link, module: Module<T>) {
        self.lookup.insert(link.clone(), module.id);
        self.modules.insert(link, module);
    }

    pub fn with_modules<R>(
        self,
        modules: HashMap<Link, super::Module<R>>,
        verbose: bool,
    ) -> Base<R> {
        Base {
            modules,
            verbose,
            lookup: self.lookup,
            ambient: self.ambient,
        }
    }

    pub const fn is_verbose(&self) -> bool {
        self.verbose
    }
}

impl<T> Base<T>
where
    T: Clone,
{
    pub fn enrich(&self, root_dir: String, failure: report::Failure) -> report::Report {
        match failure {
            report::Failure::Execution(errors) => report::Report::Execution {
                errors,

                context: report::ErrorContext {
                    root_dir,

                    modules: self
                        .modules
                        .iter()
                        .map(|(link, module)| (module.id, (link.clone(), module.text.clone())))
                        .collect(),

                    nodes: self
                        .modules
                        .values()
                        .flat_map(|module| {
                            let visitor = Visitor::new(module.id);

                            match module.ast.clone() {
                                Ast::Program(x) => x.walk(visitor),
                                Ast::Typings(x) => x.walk(visitor),
                            }
                            .1
                            .nodes
                            .into_iter()
                        })
                        .collect(),
                },
            },
        }
    }
}

struct Visitor<T> {
    _context: PhantomData<T>,
    namespace_id: NamespaceId,
    node_id: Incrementor,
    nodes: HashMap<CanonicalId, Range>,
}

impl<T> Visitor<T> {
    pub fn new(namespace_id: NamespaceId) -> Self {
        Self {
            _context: PhantomData,
            namespace_id,
            node_id: Default::default(),
            nodes: Default::default(),
        }
    }

    pub fn bind(mut self, range: Range) -> ((), Self) {
        self.nodes.insert(
            CanonicalId(self.namespace_id, NodeId(self.node_id.increment())),
            range,
        );
        ((), self)
    }
}

impl<T> CommonVisitor for Visitor<T> {
    type Context = (Range, T);
    type Binding = ();
    type TypeExpression = ();

    fn binding(self, _: lang::ast::Binding, _: Range) -> (Self::Binding, Self) {
        ((), self)
    }

    fn type_expression(
        self,
        _: lang::ast::TypeExpression<Self::Binding, Self::TypeExpression>,
        c: Self::Context,
    ) -> (Self::TypeExpression, Self) {
        self.bind(c.0)
    }
}

impl<T> ProgramVisitor for Visitor<T> {
    type Expression = ();
    type Statement = ();
    type Attribute = ();
    type Component = ();
    type Parameter = ();
    type Declaration = ();
    type Import = ();
    type Module = ();

    fn expression(
        self,
        _: lang::ast::Expression<Self::Expression, Self::Statement, Self::Component>,
        c: Self::Context,
    ) -> (Self::Expression, Self) {
        self.bind(c.0)
    }

    fn statement(
        self,
        _: lang::ast::Statement<Self::Expression>,
        c: Self::Context,
    ) -> (Self::Statement, Self) {
        self.bind(c.0)
    }

    fn attribute(
        self,
        _: lang::ast::Attribute<Self::Expression>,
        c: Self::Context,
    ) -> (Self::Attribute, Self) {
        self.bind(c.0)
    }

    fn component(
        self,
        _: lang::ast::Component<Self::Component, Self::Expression, Self::Attribute>,
        c: Self::Context,
    ) -> (Self::Component, Self) {
        self.bind(c.0)
    }

    fn parameter(
        self,
        _: lang::ast::Parameter<Self::Binding, Self::Expression, Self::TypeExpression>,
        c: Self::Context,
    ) -> (Self::Parameter, Self) {
        self.bind(c.0)
    }

    fn declaration(
        self,
        _: lang::ast::Declaration<
            Self::Binding,
            Self::Expression,
            Self::TypeExpression,
            Self::Parameter,
            Self::Module,
        >,
        c: Self::Context,
    ) -> (Self::Declaration, Self) {
        self.bind(c.0)
    }

    fn import(self, _: lang::ast::Import, c: Self::Context) -> (Self::Import, Self) {
        self.bind(c.0)
    }

    fn module(
        self,
        _: lang::ast::Module<Self::Import, Self::Declaration>,
        c: Self::Context,
    ) -> (Self::Module, Self) {
        self.bind(c.0)
    }
}

impl<T> TypingsVisitor for Visitor<T> {
    type TypeDeclaration = ();
    type TypeModule = ();

    fn type_declaration(
        self,
        _: lang::ast::TypeDeclaration<Self::Binding, Self::TypeExpression, Self::TypeModule>,
        c: Self::Context,
    ) -> (Self::TypeDeclaration, Self) {
        self.bind(c.0)
    }

    fn type_module(
        self,
        _: lang::ast::TypeModule<Self::TypeDeclaration>,
        c: Self::Context,
    ) -> (Self::TypeModule, Self) {
        self.bind(c.0)
    }
}
