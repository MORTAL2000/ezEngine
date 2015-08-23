#include <PCH.h>
#include <EditorFramework/DocumentWindow3D/3DViewWidget.moc.h>
#include <EditorFramework/DocumentWindow3D/DocumentWindow3D.moc.h>
#include <EditorFramework/DocumentWindow3D/EditorInputContext.h>

#include <QPaintEvent>

void ez3DViewWidget::paintEvent(QPaintEvent* event)
{
  //event->accept();

}

bool ez3DViewWidget::eventFilter(QObject* object, QEvent* event)
{
  if (event->type() == QEvent::Type::ShortcutOverride)
  {
    if (ezEditorInputContext::IsAnyInputContextActive())
    {
      // if the active input context does not like other shortcuts,
      // accept this event and thus block further shortcut processing
      // instead Qt will then send a keypress event
      if (ezEditorInputContext::GetActiveInputContext()->GetShortcutsDisabled())
        event->accept();
    }
  }

  return false;
}

void ez3DViewWidget::resizeEvent(QResizeEvent* event)
{
  m_pDocumentWindow->TriggerRedraw();
}

ez3DViewWidget::ez3DViewWidget(QWidget* pParent, ezDocumentWindow3D* pDocument)
  : QWidget(pParent)
  , m_pDocumentWindow(pDocument)
{
  setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  //setAttribute(Qt::WA_OpaquePaintEvent);
  setAutoFillBackground(false);
  setMouseTracking(true);

  setAttribute(Qt::WA_PaintOnScreen, true);
  setAttribute(Qt::WA_NativeWindow, true);
  setAttribute(Qt::WA_NoBackground);
  setAttribute(Qt::WA_NoSystemBackground);

  installEventFilter(this);
}

void ez3DViewWidget::keyReleaseEvent(QKeyEvent* e)
{
  // if a context is active, it gets exclusive access to the input data
  if (ezEditorInputContext::IsAnyInputContextActive())
  {
    if (ezEditorInputContext::GetActiveInputContext()->keyReleaseEvent(e))
      return;
  }

  if (ezEditorInputContext::IsAnyInputContextActive())
    return;

  // if no context is active, pass the input through in a certain order, until someone handles it
  for (auto pContext : m_InputContexts)
  {
    if (pContext->keyReleaseEvent(e))
      return;
  }

  QWidget::keyReleaseEvent(e);
}

void ez3DViewWidget::keyPressEvent(QKeyEvent* e)
{
  // if a context is active, it gets exclusive access to the input data
  if (ezEditorInputContext::IsAnyInputContextActive())
  {
    if (ezEditorInputContext::GetActiveInputContext()->keyPressEvent(e))
      return;
  }

  if (ezEditorInputContext::IsAnyInputContextActive())
    return;

  // if no context is active, pass the input through in a certain order, until someone handles it
  for (auto pContext : m_InputContexts)
  {
    if (pContext->keyPressEvent(e))
      return;
  }

  QWidget::keyPressEvent(e);
}

void ez3DViewWidget::mousePressEvent(QMouseEvent* e)
{
  // if a context is active, it gets exclusive access to the input data
  if (ezEditorInputContext::IsAnyInputContextActive())
  {
    if (ezEditorInputContext::GetActiveInputContext()->mousePressEvent(e))
      return;
  }

  if (ezEditorInputContext::IsAnyInputContextActive())
    return;

  // if no context is active, pass the input through in a certain order, until someone handles it
  for (auto pContext : m_InputContexts)
  {
    if (pContext->mousePressEvent(e))
      return;
  }

  QWidget::mousePressEvent(e);
}

void ez3DViewWidget::mouseReleaseEvent(QMouseEvent* e)
{
  // if a context is active, it gets exclusive access to the input data
  if (ezEditorInputContext::IsAnyInputContextActive())
  {
    if (ezEditorInputContext::GetActiveInputContext()->mouseReleaseEvent(e))
      return;
  }

  if (ezEditorInputContext::IsAnyInputContextActive())
    return;

  // if no context is active, pass the input through in a certain order, until someone handles it
  for (auto pContext : m_InputContexts)
  {
    if (pContext->mouseReleaseEvent(e))
      return;
  }

  QWidget::mouseReleaseEvent(e);
}

void ez3DViewWidget::mouseMoveEvent(QMouseEvent* e)
{
  // if a context is active, it gets exclusive access to the input data
  if (ezEditorInputContext::IsAnyInputContextActive())
  {
    if (ezEditorInputContext::GetActiveInputContext()->mouseMoveEvent(e))
      return;
  }

  if (ezEditorInputContext::IsAnyInputContextActive())
    return;

  // if no context is active, pass the input through in a certain order, until someone handles it
  for (auto pContext : m_InputContexts)
  {
    if (pContext->mouseMoveEvent(e))
      return;
  }

  QWidget::mouseMoveEvent(e);
}

void ez3DViewWidget::wheelEvent(QWheelEvent* e)
{
  // if a context is active, it gets exclusive access to the input data
  if (ezEditorInputContext::IsAnyInputContextActive())
  {
    if (ezEditorInputContext::GetActiveInputContext()->wheelEvent(e))
      return;
  }

  if (ezEditorInputContext::IsAnyInputContextActive())
    return;

  // if no context is active, pass the input through in a certain order, until someone handles it
  for (auto pContext : m_InputContexts)
  {
    if (pContext->wheelEvent(e))
      return;
  }

  QWidget::wheelEvent(e);
}

void ez3DViewWidget::focusOutEvent(QFocusEvent* e)
{
  if (ezEditorInputContext::IsAnyInputContextActive())
  {
    ezEditorInputContext::GetActiveInputContext()->FocusLost();
    ezEditorInputContext::SetActiveInputContext(nullptr);
  }

  QWidget::focusOutEvent(e);
}