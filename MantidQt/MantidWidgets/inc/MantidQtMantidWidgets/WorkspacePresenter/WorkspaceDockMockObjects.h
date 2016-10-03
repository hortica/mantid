#ifndef MANTIDQT_MANTIDWIDGETS_WORKSPACEDOCKMOCKOBJECTS_H
#define MANTIDQT_MANTIDWIDGETS_WORKSPACEDOCKMOCKOBJECTS_H

#include "MantidKernel/WarningSuppressions.h"
#include "MantidKernel/make_unique.h"
#include <MantidAPI/Workspace.h>
#include <MantidQtMantidWidgets/WorkspacePresenter/IWorkspaceDockView.h>
#include <MantidQtMantidWidgets/WorkspacePresenter/WorkspacePresenter.h>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <gmock/gmock.h>

using namespace MantidQt::MantidWidgets;

GCC_DIAG_OFF_SUGGEST_OVERRIDE

class MockWorkspaceDockView
    : public IWorkspaceDockView,
      public boost::enable_shared_from_this<MockWorkspaceDockView> {
public:
  MockWorkspaceDockView() {}
  ~MockWorkspaceDockView() override {}

  void init() override {
    auto presenter = boost::make_shared<WorkspacePresenter>(shared_from_this());
    m_presenter = boost::dynamic_pointer_cast<ViewNotifiable>(presenter);
    presenter->init();
  }

  MOCK_CONST_METHOD2(askUserYesNo, bool(const std::string &caption,
                                        const std::string &message));
  MOCK_CONST_METHOD2(showCriticalUserMessage, void(const std::string &caption,
                                                   const std::string &message));

  MOCK_METHOD0(showLoadDialog, void());
  MOCK_METHOD0(showLiveDataDialog, void());
  MOCK_CONST_METHOD0(isPromptDelete, bool());
  MOCK_CONST_METHOD0(deleteConfirmation, bool());
  MOCK_METHOD1(deleteWorkspaces, void(const StringList &wsNames));
  MOCK_METHOD0(clearView, void());
  MOCK_METHOD2(recordWorkspaceRename,
               void(const std::string &oldName, const std::string &newName));
  MOCK_METHOD1(showRenameDialog, void(const StringList &wsNames));
  MOCK_CONST_METHOD2(groupWorkspaces, void(const StringList &wsNames,
                                           const std::string &groupName));
  MOCK_CONST_METHOD1(ungroupWorkspaces, void(const StringList &wsNames));
  MOCK_CONST_METHOD0(getSortDirection, SortDirection());
  MOCK_CONST_METHOD0(getSortCriteria, SortCriteria());
  MOCK_METHOD2(sortWorkspaces,
               void(IWorkspaceDockView::SortCriteria criteria,
                    IWorkspaceDockView::SortDirection direction));
  MOCK_CONST_METHOD0(getSaveFileType, SaveFileType());
  MOCK_METHOD1(saveWorkspace, void(SaveFileType type));
  MOCK_METHOD1(saveWorkspaces, void(const StringList &wsNames));
  MOCK_METHOD1(
      updateTree,
      void(const std::map<std::string, Mantid::API::Workspace_sptr> &items));

  MOCK_CONST_METHOD0(getFilterText, std::string());
  MOCK_METHOD1(filterWorkspaces, void(const std::string &filterText));

  MOCK_CONST_METHOD0(getSelectedWorkspaceNames, StringList());
  MOCK_CONST_METHOD0(getSelectedWorkspace, Mantid::API::Workspace_sptr());

  // Context Menu Handlers
  MOCK_METHOD0(popupContextMenu, void());
  MOCK_METHOD0(showWorkspaceData, void());
  MOCK_METHOD0(showInstrumentView, void());
  MOCK_METHOD0(saveToProgram, void());
  MOCK_METHOD1(plotSpectrum, void(bool showErrors));
  MOCK_METHOD0(showColourFillPlot, void());
  MOCK_METHOD0(showDetectorsTable, void());
  MOCK_METHOD0(showBoxDataTable, void());
  MOCK_METHOD0(showVatesGUI, void());
  MOCK_METHOD0(showMDPlot, void());
  MOCK_METHOD0(showListData, void());
  MOCK_METHOD0(showSpectrumViewer, void());
  MOCK_METHOD0(showSliceViewer, void());
  MOCK_METHOD0(showLogs, void());
  MOCK_METHOD0(showSampleMaterialWindow, void());
  MOCK_METHOD0(showAlgorithmHistory, void());
  MOCK_METHOD0(showTransposed, void());
  MOCK_METHOD0(convertToMatrixWorkspace, void());
  MOCK_METHOD0(convertMDHistoToMatrixWorkspace, void());
  MOCK_METHOD0(clearUBMatrix, void());
  MOCK_METHOD0(showSurfacePlot, void());
  MOCK_METHOD0(showContourPlot, void());

  // Methods which are not to be mocked
  void enableDeletePrompt(bool) override {}
  WorkspacePresenterWN_wptr getPresenterWeakPtr() override {
    return boost::dynamic_pointer_cast<WorkspacePresenter>(m_presenter);
  }

  void refreshWorkspaces() override {}

  WorkspacePresenterVN_sptr getPresenterSharedPtr() { return m_presenter; }

private:
  WorkspacePresenterVN_sptr m_presenter;
};

class MockWorkspaceProviderNotifiable : public WorkspaceProviderNotifiable {
public:
  ~MockWorkspaceProviderNotifiable() override {}
  MOCK_METHOD1(notifyFromWorkspaceProvider, void(Flag flag));
};

GCC_DIAG_ON_SUGGEST_OVERRIDE

#endif // MANTIDQT_MANTIDWIDGETS_WORKSPACEDOCKMOCKOBJECTS_H