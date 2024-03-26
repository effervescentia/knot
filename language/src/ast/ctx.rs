use super::walk::{IntoSpan, Walk};
use crate::{FragmentMap, Node, Range};

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

impl<Visitor, Context> Walk<Visitor> for Expression<Context>
where
    Visitor: super::walk::Visit,
{
    type Output = Visitor::Expression;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.into_span().walk(v)
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct Statement<Context>(pub Node<super::Statement<Expression<Context>>, Context>);

impl Statement<()> {
    pub const fn raw(x: super::Statement<Expression<()>>, range: Range) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: super::Statement<Expression<()>>) -> Self {
        Self::raw(x, Range::nil())
    }
}

impl<Visitor, Context> Walk<Visitor> for Statement<Context>
where
    Visitor: super::walk::Visit,
{
    type Output = Visitor::Statement;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.into_span().walk(v)
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct Component<Context>(
    pub Node<super::Component<Component<Context>, Expression<Context>>, Context>,
);

impl Component<()> {
    pub const fn raw(x: super::Component<Self, Expression<()>>, range: Range) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: super::Component<Self, Expression<()>>) -> Self {
        Self::raw(x, Range::nil())
    }
}

impl<Visitor, Context> Walk<Visitor> for Component<Context>
where
    Visitor: super::walk::Visit,
{
    type Output = Visitor::Component;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.into_span().walk(v)
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct TypeExpression<Context>(
    pub Node<super::TypeExpression<TypeExpression<Context>>, Context>,
);

impl TypeExpression<()> {
    pub const fn raw(x: super::TypeExpression<Self>, range: Range) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: super::TypeExpression<Self>) -> Self {
        Self::raw(x, Range::nil())
    }
}

impl<Visitor, Context> Walk<Visitor> for TypeExpression<Context>
where
    Visitor: super::walk::Visit,
{
    type Output = Visitor::TypeExpression;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.into_span().walk(v)
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct Parameter<Context>(
    pub Node<super::Parameter<Binding, Expression<Context>, TypeExpression<Context>>, Context>,
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

impl<Visitor, Context> Walk<Visitor> for Parameter<Context>
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
pub struct Declaration<Context>(
    pub  Node<
        super::Declaration<
            Binding,
            Expression<Context>,
            TypeExpression<Context>,
            Parameter<Context>,
            Module<Context>,
        >,
        Context,
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

impl<Visitor, Context> Walk<Visitor> for Declaration<Context>
where
    Visitor: super::walk::Visit,
{
    type Output = Visitor::Declaration;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.into_span().walk(v)
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct Import<Context>(pub Node<super::Import, Context>);

impl Import<()> {
    pub const fn raw(x: super::Import, range: Range) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: super::Import) -> Self {
        Self::raw(x, Range::nil())
    }
}

impl<Visitor, Context> Walk<Visitor> for Import<Context>
where
    Visitor: super::walk::Visit,
{
    type Output = Visitor::Import;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.into_span().walk(v)
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct Module<Context>(pub Node<super::Module<Import<Context>, Declaration<Context>>, Context>);

impl Module<()> {
    pub const fn raw(x: super::Module<Import<()>, Declaration<()>>, range: Range) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: super::Module<Import<()>, Declaration<()>>) -> Self {
        Self::raw(x, Range::nil())
    }
}

impl<Visitor, Context> Walk<Visitor> for Module<Context>
where
    Visitor: super::walk::Visit,
{
    type Output = Visitor::Module;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.into_span().walk(v)
    }
}

impl<Context> super::into_fragments::IntoFragments for Module<Context> {
    fn into_fragments(self) -> FragmentMap {
        self.walk(super::into_fragments::Visitor::default())
            .1
            .fragments()
    }
}
