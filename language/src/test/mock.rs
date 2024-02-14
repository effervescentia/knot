use crate::{
    ast::{
        self, explode,
        walk::{self, Visit, Walk},
    },
    FragmentMap, Range,
};

impl<Value> walk::Span<Value> {
    fn mock(x: Value) -> Self {
        Self(x, Range::nil())
    }
}

pub struct Binding(pub walk::Span<ast::Binding>);

impl Binding {
    pub fn new(x: &str) -> Self {
        Self(walk::Span::mock(ast::Binding(x.to_owned())))
    }
}

impl<Visitor> walk::Walk<Visitor> for Binding
where
    Visitor: Visit,
{
    type Output = Visitor::Binding;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.walk(v)
    }
}

pub struct Expression(pub walk::Span<ast::Expression<Expression, Statement, Component>>);

impl Expression {
    pub fn new(x: ast::Expression<Self, Statement, Component>) -> Self {
        Self(walk::Span::mock(x))
    }
}

impl<Visitor> walk::Walk<Visitor> for Expression
where
    Visitor: Visit,
{
    type Output = Visitor::Expression;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.walk(v)
    }
}

pub struct Statement(pub walk::Span<ast::Statement<Expression>>);

impl Statement {
    pub fn new(x: ast::Statement<Expression>) -> Self {
        Self(walk::Span::mock(x))
    }
}

impl<Visitor> walk::Walk<Visitor> for Statement
where
    Visitor: Visit,
{
    type Output = Visitor::Statement;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.walk(v)
    }
}

pub struct Component(pub walk::Span<ast::Component<Component, Expression>>);

impl Component {
    pub fn new(x: ast::Component<Self, Expression>) -> Self {
        Self(walk::Span::mock(x))
    }
}

impl<Visitor> walk::Walk<Visitor> for Component
where
    Visitor: Visit,
{
    type Output = Visitor::Component;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.walk(v)
    }
}

pub struct TypeExpression(pub walk::Span<ast::TypeExpression<TypeExpression>>);

impl TypeExpression {
    pub fn new(x: ast::TypeExpression<Self>) -> Self {
        Self(walk::Span::mock(x))
    }
}

impl<Visitor> walk::Walk<Visitor> for TypeExpression
where
    Visitor: Visit,
{
    type Output = Visitor::TypeExpression;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.walk(v)
    }
}

pub struct Parameter(pub walk::Span<ast::Parameter<Binding, Expression, TypeExpression>>);

impl Parameter {
    pub fn new(x: ast::Parameter<Binding, Expression, TypeExpression>) -> Self {
        Self(walk::Span::mock(x))
    }
}

impl<Visitor> walk::Walk<Visitor> for Parameter
where
    Visitor: Visit,
{
    type Output = Visitor::Parameter;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.walk(v)
    }
}

pub struct Declaration(
    pub walk::Span<ast::Declaration<Binding, Expression, TypeExpression, Parameter, Module>>,
);

impl Declaration {
    pub fn new(
        x: ast::Declaration<Binding, Expression, TypeExpression, Parameter, Module>,
    ) -> Self {
        Self(walk::Span::mock(x))
    }
}

impl<Visitor> walk::Walk<Visitor> for Declaration
where
    Visitor: Visit,
{
    type Output = Visitor::Declaration;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.walk(v)
    }
}

pub struct Import(pub walk::Span<ast::Import>);

impl Import {
    pub fn new(x: ast::Import) -> Self {
        Self(walk::Span::mock(x))
    }
}

impl<Visitor> walk::Walk<Visitor> for Import
where
    Visitor: Visit,
{
    type Output = Visitor::Import;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.walk(v)
    }
}

pub struct Module(pub walk::Span<ast::Module<Import, Declaration>>);

impl Module {
    pub fn new(x: ast::Module<Import, Declaration>) -> Self {
        Self(walk::Span::mock(x))
    }
}

impl<Visitor> walk::Walk<Visitor> for Module
where
    Visitor: Visit,
{
    type Output = Visitor::Module;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.walk(v)
    }
}

impl explode::Explode for Module {
    fn explode(self) -> FragmentMap {
        self.walk(explode::Visitor::default()).1.fragments()
    }
}
