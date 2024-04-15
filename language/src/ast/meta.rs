use super::shape;
use crate::{
    walk::{Visit, Walk},
    Node, Range,
};
use std::fmt::Display;

/* binding */

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

    pub fn name(&self) -> &str {
        &self.0.value().0
    }
}

impl<Visitor, Meta> Walk<Visitor> for Binding
where
    Visitor: Visit<Context = (Range, Meta)>,
{
    type Output = Visitor::Binding;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.walk(v)
    }
}

/* expression */

type ExpressionValue<Meta> = super::Expression<Expression<Meta>, Statement<Meta>, Component<Meta>>;

#[allow(clippy::type_complexity)]
#[derive(Clone, Debug, PartialEq)]
pub struct Expression<Meta>(pub Node<ExpressionValue<Meta>, Meta>);

impl<Meta> Expression<Meta> {
    pub fn typed(v: ExpressionValue<Meta>, m: Meta) -> Self {
        Self(Node::typed(v, m))
    }
}

impl Expression<()> {
    pub const fn raw(x: ExpressionValue<()>, range: Range) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: ExpressionValue<()>) -> Self {
        Self::raw(x, Range::nil())
    }
}

impl<Visitor, Meta> Walk<Visitor> for Expression<Meta>
where
    Visitor: Visit<Context = (Range, Meta)>,
{
    type Output = Visitor::Expression;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.walk(v)
    }
}

/* statement */

type StatementValue<Meta> = super::Statement<Expression<Meta>>;

#[derive(Clone, Debug, PartialEq)]
pub struct Statement<Meta>(pub Node<StatementValue<Meta>, Meta>);

impl<Meta> Statement<Meta> {
    pub fn typed(v: StatementValue<Meta>, m: Meta) -> Self {
        Self(Node::typed(v, m))
    }
}

impl Statement<()> {
    pub const fn raw(x: StatementValue<()>, range: Range) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: StatementValue<()>) -> Self {
        Self::raw(x, Range::nil())
    }
}

impl<Visitor, Meta> Walk<Visitor> for Statement<Meta>
where
    Visitor: Visit<Context = (Range, Meta)>,
{
    type Output = Visitor::Statement;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.walk(v)
    }
}

/* component */

type ComponentValue<Meta> = super::Component<Component<Meta>, Expression<Meta>>;

#[derive(Clone, Debug, PartialEq)]
pub struct Component<Meta>(pub Node<ComponentValue<Meta>, Meta>);

impl<Meta> Component<Meta> {
    pub fn typed(v: ComponentValue<Meta>, m: Meta) -> Self {
        Self(Node::typed(v, m))
    }
}

impl Component<()> {
    pub const fn raw(x: ComponentValue<()>, range: Range) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: ComponentValue<()>) -> Self {
        Self::raw(x, Range::nil())
    }
}

impl<Visitor, Meta> Walk<Visitor> for Component<Meta>
where
    Visitor: Visit<Context = (Range, Meta)>,
{
    type Output = Visitor::Component;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.walk(v)
    }
}

/* type expression */

#[derive(Clone, Debug, PartialEq)]
pub struct TypeExpression<Meta>(pub Node<super::TypeExpression<Binding, Self>, Meta>);

impl<Meta> TypeExpression<Meta> {
    pub fn typed(v: super::TypeExpression<Binding, Self>, m: Meta) -> Self {
        Self(Node::typed(v, m))
    }
}

impl TypeExpression<()> {
    pub const fn raw(x: super::TypeExpression<Binding, Self>, range: Range) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: super::TypeExpression<Binding, Self>) -> Self {
        Self::raw(x, Range::nil())
    }
}

impl<Visitor, Meta> Walk<Visitor> for TypeExpression<Meta>
where
    Visitor: Visit<Context = (Range, Meta)>,
{
    type Output = Visitor::TypeExpression;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.walk(v)
    }
}

/* parameter */

type ParameterValue<Meta> = super::Parameter<Binding, Expression<Meta>, TypeExpression<Meta>>;

#[derive(Clone, Debug, PartialEq)]
pub struct Parameter<Meta>(pub Node<ParameterValue<Meta>, Meta>);

impl<Meta> Parameter<Meta> {
    pub fn typed(v: ParameterValue<Meta>, m: Meta) -> Self {
        Self(Node::typed(v, m))
    }
}

impl Parameter<()> {
    pub const fn raw(x: ParameterValue<()>, range: Range) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: ParameterValue<()>) -> Self {
        Self::raw(x, Range::nil())
    }
}

impl<Visitor, Meta> Walk<Visitor> for Parameter<Meta>
where
    Visitor: Visit<Context = (Range, Meta)>,
{
    type Output = Visitor::Parameter;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.walk(v)
    }
}

/* declaration */

type DeclarationValue<Meta> = super::Declaration<
    Binding,
    Expression<Meta>,
    TypeExpression<Meta>,
    Parameter<Meta>,
    Module<Meta>,
>;

#[allow(clippy::type_complexity)]
#[derive(Clone, Debug, PartialEq)]
pub struct Declaration<Meta>(pub Node<DeclarationValue<Meta>, Meta>);

impl<Meta> Declaration<Meta> {
    pub fn typed(v: DeclarationValue<Meta>, m: Meta) -> Self {
        Self(Node::typed(v, m))
    }
}

impl Declaration<()> {
    pub const fn raw(x: DeclarationValue<()>, range: Range) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: DeclarationValue<()>) -> Self {
        Self::raw(x, Range::nil())
    }
}

impl<Visitor, Meta> Walk<Visitor> for Declaration<Meta>
where
    Visitor: Visit<Context = (Range, Meta)>,
{
    type Output = Visitor::Declaration;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.walk(v)
    }
}

/* import */

#[derive(Clone, Debug, PartialEq)]
pub struct Import<Meta>(pub Node<super::Import, Meta>);

impl<Meta> Import<Meta> {
    pub fn typed(v: super::Import, m: Meta) -> Self {
        Self(Node::typed(v, m))
    }
}

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
    Visitor: Visit<Context = (Range, Meta)>,
{
    type Output = Visitor::Import;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.walk(v)
    }
}

/* module */

type ModuleValue<Meta> = super::Module<Import<Meta>, Declaration<Meta>>;

#[derive(Clone, Debug, PartialEq)]
pub struct Module<Meta>(pub Node<ModuleValue<Meta>, Meta>);

impl<Meta> Module<Meta> {
    pub fn typed(v: ModuleValue<Meta>, m: Meta) -> Self {
        Self(Node::typed(v, m))
    }
}

impl Module<()> {
    pub const fn raw(x: ModuleValue<()>, range: Range) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: ModuleValue<()>) -> Self {
        Self::raw(x, Range::nil())
    }
}

impl<Visitor, Meta> Walk<Visitor> for Module<Meta>
where
    Visitor: Visit<Context = (Range, Meta)>,
{
    type Output = Visitor::Module;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.0.walk(v)
    }
}

/* program */

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
        shape::Program(self.0.walk(super::shape::Visitor::default()).0)
    }
}

impl<Meta> Display for Program<Meta>
where
    Meta: Clone,
{
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        self.0
            .clone()
            .walk(super::shape::Visitor::default())
            .0
            .fmt(f)
    }
}

/* type declaration */

type TypeDeclarationValue<Meta> = super::TypeDeclaration<Binding, TypeExpression<Meta>>;

#[derive(Clone, Debug, PartialEq)]
pub struct TypeDeclaration<Meta>(pub Node<TypeDeclarationValue<Meta>, Meta>);

impl<Meta> TypeDeclaration<Meta> {
    pub fn typed(v: TypeDeclarationValue<Meta>, m: Meta) -> Self {
        Self(Node::typed(v, m))
    }
}

impl TypeDeclaration<()> {
    pub const fn raw(x: TypeDeclarationValue<()>, range: Range) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: TypeDeclarationValue<()>) -> Self {
        Self::raw(x, Range::nil())
    }
}

// impl<Visitor, Meta> Walk<Visitor> for TypeDeclaration<Meta>
// where
//     Visitor: Visit<Context = (Range, Meta)>,
// {
//     type Output = Visitor::TypeDeclaration;

//     fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
//         self.0.walk(v)
//     }
// }

/* type module */

type TypeModuleValue<Meta> = super::TypeModule<TypeDeclaration<Meta>>;

#[derive(Clone, Debug, PartialEq)]
pub struct TypeModule<Meta>(pub Node<TypeModuleValue<Meta>, Meta>);

impl<Meta> TypeModule<Meta> {
    pub fn typed(v: TypeModuleValue<Meta>, m: Meta) -> Self {
        Self(Node::typed(v, m))
    }
}

impl TypeModule<()> {
    pub const fn raw(x: TypeModuleValue<()>, range: Range) -> Self {
        Self(Node::raw(x, range))
    }

    #[cfg(feature = "test")]
    pub fn mock(x: TypeModuleValue<()>) -> Self {
        Self::raw(x, Range::nil())
    }
}

// impl<Visitor, Meta> Walk<Visitor> for TypeModule<Meta>
// where
//     Visitor: Visit<Context = (Range, Meta)>,
// {
//     type Output = Visitor::Module;

//     fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
//         self.0.walk(v)
//     }
// }

/* typings */

#[derive(Clone, Debug, PartialEq)]
pub struct Typings<Meta>(pub TypeModule<Meta>);

impl<Meta> Typings<Meta> {
    pub const fn node(&self) -> &Node<super::TypeModule<TypeDeclaration<Meta>>, Meta> {
        let Self(module) = self;
        &module.0
    }

    // pub fn to_shape(self) -> shape::Typings {
    //     shape::Typings(self.0.walk(super::shape::Visitor::default()).0)
    // }
}

// impl<Meta> Display for Typings<Meta>
// where
//     Meta: Clone,
// {
//     fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
//         self.0
//             .clone()
//             .walk(super::shape::Visitor::default())
//             .0
//             .fmt(f)
//     }
// }
