use super::{
    shape,
    walk::{IntoSpan, Walk},
};
use crate::{Node, Range};
use std::fmt::Display;

#[derive(Clone, Debug, PartialEq)]
pub struct Binding(pub Node<super::Binding, ()>);

impl Binding {
    pub const fn new(x: super::Binding, range: Range) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: &str) -> Self {
        Self::new(super::Binding(x.to_owned()), Range::nil())
    }
}

impl<Visitor> Walk<Visitor> for Binding
where
    Visitor: super::walk::Visit,
{
    type Output = Visitor::Binding;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.into_span().walk(v)
    }
}

#[allow(clippy::type_complexity)]
#[derive(Clone, Debug, PartialEq)]
pub struct Expression<Context>(
    pub  Node<
        super::Expression<Expression<Context>, Statement<Context>, Component<Context>>,
        Context,
    >,
);

impl Expression<()> {
    pub const fn raw(
        x: super::Expression<Self, Statement<()>, Component<()>>,
        range: Range,
    ) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: super::Expression<Self, Statement<()>, Component<()>>) -> Self {
        Self::raw(x, Range::nil())
    }
}

impl<Visitor, Meta> Walk<Visitor> for Expression<Meta>
where
    Visitor: super::walk::Visit,
{
    type Output = Visitor::Expression;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.into_span().walk(v)
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct Statement<Meta>(pub Node<super::Statement<Expression<Meta>>, Meta>);

impl Statement<()> {
    pub const fn raw(x: super::Statement<Expression<()>>, range: Range) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: super::Statement<Expression<()>>) -> Self {
        Self::raw(x, Range::nil())
    }
}

impl<Visitor, Meta> Walk<Visitor> for Statement<Meta>
where
    Visitor: super::walk::Visit,
{
    type Output = Visitor::Statement;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.into_span().walk(v)
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct Component<Meta>(pub Node<super::Component<Component<Meta>, Expression<Meta>>, Meta>);

impl Component<()> {
    pub const fn raw(x: super::Component<Self, Expression<()>>, range: Range) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: super::Component<Self, Expression<()>>) -> Self {
        Self::raw(x, Range::nil())
    }
}

impl<Visitor, Meta> Walk<Visitor> for Component<Meta>
where
    Visitor: super::walk::Visit,
{
    type Output = Visitor::Component;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.into_span().walk(v)
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct TypeExpression<Meta>(pub Node<super::TypeExpression<TypeExpression<Meta>>, Meta>);

impl TypeExpression<()> {
    pub const fn raw(x: super::TypeExpression<Self>, range: Range) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: super::TypeExpression<Self>) -> Self {
        Self::raw(x, Range::nil())
    }
}

impl<Visitor, Meta> Walk<Visitor> for TypeExpression<Meta>
where
    Visitor: super::walk::Visit,
{
    type Output = Visitor::TypeExpression;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.into_span().walk(v)
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct Parameter<Meta>(
    pub Node<super::Parameter<Binding, Expression<Meta>, TypeExpression<Meta>>, Meta>,
);

impl Parameter<()> {
    pub const fn raw(
        x: super::Parameter<Binding, Expression<()>, TypeExpression<()>>,
        range: Range,
    ) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: super::Parameter<Binding, Expression<()>, TypeExpression<()>>) -> Self {
        Self::raw(x, Range::nil())
    }
}

impl<Visitor, Meta> Walk<Visitor> for Parameter<Meta>
where
    Visitor: super::walk::Visit,
{
    type Output = Visitor::Parameter;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.into_span().walk(v)
    }
}

#[allow(clippy::type_complexity)]
#[derive(Clone, Debug, PartialEq)]
pub struct Declaration<Meta>(
    pub  Node<
        super::Declaration<
            Binding,
            Expression<Meta>,
            TypeExpression<Meta>,
            Parameter<Meta>,
            Module<Meta>,
        >,
        Meta,
    >,
);

impl Declaration<()> {
    pub const fn raw(
        x: super::Declaration<
            Binding,
            Expression<()>,
            TypeExpression<()>,
            Parameter<()>,
            Module<()>,
        >,
        range: Range,
    ) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(
        x: super::Declaration<
            Binding,
            Expression<()>,
            TypeExpression<()>,
            Parameter<()>,
            Module<()>,
        >,
    ) -> Self {
        Self::raw(x, Range::nil())
    }
}

impl<Visitor, Meta> Walk<Visitor> for Declaration<Meta>
where
    Visitor: super::walk::Visit,
{
    type Output = Visitor::Declaration;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.into_span().walk(v)
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct Import<Meta>(pub Node<super::Import, Meta>);

impl Import<()> {
    pub const fn raw(x: super::Import, range: Range) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: super::Import) -> Self {
        Self::raw(x, Range::nil())
    }
}

impl<Visitor, Meta> Walk<Visitor> for Import<Meta>
where
    Visitor: super::walk::Visit,
{
    type Output = Visitor::Import;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.into_span().walk(v)
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct Module<Meta>(pub Node<super::Module<Import<Meta>, Declaration<Meta>>, Meta>);

impl Module<()> {
    pub const fn raw(x: super::Module<Import<()>, Declaration<()>>, range: Range) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: super::Module<Import<()>, Declaration<()>>) -> Self {
        Self::raw(x, Range::nil())
    }
}

impl<Visitor, Meta> Walk<Visitor> for Module<Meta>
where
    Visitor: super::walk::Visit,
{
    type Output = Visitor::Module;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.into_span().walk(v)
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct Program<Meta>(pub Module<Meta>);

impl<Meta> Program<Meta> {
    pub const fn node(&self) -> &Node<super::Module<Import<Meta>, Declaration<Meta>>, Meta> {
        let Self(module) = self;
        &module.0
    }

    pub const fn imports(&self) -> &Vec<Import<Meta>> {
        let Self(Module(Node(super::Module { imports, .. }, ..), ..)) = self;

        imports
    }

    pub fn to_shape(self) -> shape::Program {
        shape::Program(self.0.walk(super::shape::Visitor).0)
    }
}

impl<Meta> Display for Program<Meta>
where
    Meta: Clone,
{
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        self.0.clone().walk(super::shape::Visitor).0.fmt(f)
    }
}
