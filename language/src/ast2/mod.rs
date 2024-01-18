mod raw;
mod typed;
mod walk;

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct Range {
    start: (usize, usize),
    end: (usize, usize),
}

impl Range {
    pub const fn new(start: (usize, usize), end: (usize, usize)) -> Self {
        Self { start, end }
    }
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct Binding {
    name: String,
    range: Range,
}

impl Binding {
    pub const fn new(name: String, range: Range) -> Self {
        Self { name, range }
    }
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum UnaryOperator {
    Not,

    Negate,
    Absolute,
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum BinaryOperator {
    And,
    Or,

    Equal,
    NotEqual,

    LessThan,
    LessThanOrEqual,
    GreaterThan,
    GreaterThanOrEqual,

    Add,
    Subtract,
    Multiply,
    Divide,
    Exponent,
}

#[derive(Clone, Debug, PartialEq)]
pub enum Primitive {
    Nil,
    Boolean(bool),
    Integer(i64),
    Float(f64, i32),
    String(String),
}

// try to keep all the `Box` indirection within `Expression`
// since it is the root recursive structure
#[derive(Debug, PartialEq)]
pub enum Expression<Expr, Stmt, Comp> {
    Primitive(Primitive),
    Identifier(String),

    /* containers */
    Group(Box<Expr>),
    Closure(Vec<Stmt>),

    /* operations */
    UnaryOperation(UnaryOperator, Box<Expr>),
    BinaryOperation(BinaryOperator, Box<Expr>, Box<Expr>),
    PropertyAccess(Box<Expr>, String),
    FunctionCall(Box<Expr>, Vec<Expr>),

    /* domain syntax */
    Component(Box<Comp>),
    Style(Vec<(String, Expr)>),
}

#[derive(Debug, Eq, PartialEq)]
pub enum Statement<Expr> {
    Expression(Expr),
    Variable(Binding, Expr),
}

#[derive(Debug, Eq, PartialEq)]
pub enum Component<Expr, Comp> {
    Fragment(Vec<Comp>),
    OpenElement(String, Vec<(String, Option<Expr>)>, Vec<Comp>, String),
    ClosedElement(String, Vec<(String, Option<Expr>)>),
    Expression(Expr),
    Text(String),
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum Visibility {
    Public,
    Private,
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct Storage {
    visibility: Visibility,
    binding: Binding,
}

impl Storage {
    pub fn new(visibility: Visibility, binding: Binding) -> Self {
        Self {
            visibility,
            binding,
        }
    }
}

#[derive(Debug, Eq, PartialEq)]
pub struct Parameter<Expr, TExpr> {
    name: String,
    value_type: Option<TExpr>,
    default_value: Option<Expr>,
}

#[derive(Debug, Eq, PartialEq)]
pub enum Declaration<Expr, Param, Mod, TExpr> {
    TypeAlias {
        storage: Storage,
        value: TExpr,
    },
    Constant {
        storage: Storage,
        value_type: Option<TExpr>,
        value: Expr,
    },
    Enumerated {
        storage: Storage,
        variants: Vec<(String, Vec<TExpr>)>,
    },
    Function {
        storage: Storage,
        parameters: Vec<Param>,
        body_type: Option<TExpr>,
        body: Expr,
    },
    View {
        storage: Storage,
        parameters: Vec<Param>,
        body: Expr,
    },
    Module {
        storage: Storage,
        value: Mod,
    },
}

#[derive(Debug, Eq, PartialEq)]
pub enum TypePrimitive {
    Nil,
    Boolean,
    Integer,
    Float,
    String,
    Style,
    Component,
}

#[derive(Debug, Eq, PartialEq)]
pub enum TypeExpression<TExpr> {
    Primitive(TypePrimitive),
    Identifier(String),
    Group(Box<TExpr>),
    PropertyAccess(Box<TExpr>, String),
    Function(Vec<TExpr>, Box<TExpr>),
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum ImportSource {
    Root,
    Local,
    Named(String),
    Scoped { scope: String, name: String },
}

#[derive(Debug, Eq, PartialEq)]
pub struct Import {
    source: ImportSource,
    path: Vec<String>,
    alias: Option<String>,
}

#[derive(Debug, Eq, PartialEq)]
pub struct Module<Imp, Decl> {
    imports: Vec<Imp>,
    declarations: Vec<Decl>,
}

#[derive(Debug, Eq, PartialEq)]
pub struct Program<Module>(Module);
