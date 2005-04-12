//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// RCS-ID:      $Id$
// Copyright (C) 2002 - 2005, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
// dlgJob.cpp - PostgreSQL Job Property
//
//////////////////////////////////////////////////////////////////////////

// wxWindows headers
#include <wx/wx.h>

// App headers
#include "pgAdmin3.h"
#include "misc.h"
#include "dlgJob.h"
#include "dlgStep.h"
#include "dlgSchedule.h"
#include "pgaJob.h"
#include "pgaStep.h"
#include "pgaSchedule.h"
#include "frmMain.h"

// Images
#include "images/job.xpm"


// pointer to controls
#define	txtID				CTRL_TEXT("txtID")
#define chkEnabled          CTRL_CHECKBOX("chkEnabled")
#define cbJobclass          CTRL_COMBOBOX("cbJobclass")
#define txtCreated          CTRL_TEXT("txtCreated")
#define txtChanged          CTRL_TEXT("txtChanged")
#define txtNextrun          CTRL_TEXT("txtNextrun")
#define txtLastrun          CTRL_TEXT("txtLastrun")
#define txtLastresult       CTRL_TEXT("txtLastResult")

#define lstSteps            CTRL_LISTVIEW("lstSteps")
#define btnChangeStep       CTRL_BUTTON("btnChangeStep")
#define btnAddStep          CTRL_BUTTON("btnAddStep")
#define btnRemoveStep       CTRL_BUTTON("btnRemoveStep")

#define lstSchedules        CTRL_LISTVIEW("lstSchedules")
#define btnChangeSchedule   CTRL_BUTTON("btnChangeSchedule")
#define btnAddSchedule      CTRL_BUTTON("btnAddSchedule")
#define btnRemoveSchedule   CTRL_BUTTON("btnRemoveSchedule")


BEGIN_EVENT_TABLE(dlgJob, dlgAgentProperty)
    EVT_CHECKBOX(XRCID("chkEnabled"),               dlgProperty::OnChange)
    EVT_COMBOBOX(XRCID("cbJobclass"),               dlgProperty::OnChange)

    EVT_LIST_ITEM_SELECTED(XRCID("lstSteps"),       dlgJob::OnSelChangeStep)
    EVT_BUTTON(XRCID("btnChangeStep"),              dlgJob::OnChangeStep)
    EVT_BUTTON(XRCID("btnAddStep"),                 dlgJob::OnAddStep)
    EVT_BUTTON(XRCID("btnRemoveStep"),              dlgJob::OnRemoveStep)

    EVT_LIST_ITEM_SELECTED(XRCID("lstSchedules"),   dlgJob::OnSelChangeSchedule)
    EVT_BUTTON(XRCID("btnChangeSchedule"),          dlgJob::OnChangeSchedule)
    EVT_BUTTON(XRCID("btnAddSchedule"),             dlgJob::OnAddSchedule)
    EVT_BUTTON(XRCID("btnRemoveSchedule"),          dlgJob::OnRemoveSchedule)
END_EVENT_TABLE();


dlgJob::dlgJob(frmMain *frame, pgaJob *node)
: dlgAgentProperty(frame, wxT("dlgJob"))
{
    SetIcon(wxIcon(job_xpm));
    job=node;

	txtID->Disable();
    txtCreated->Disable();
    txtChanged->Disable();
    txtNextrun->Disable();
    txtLastrun->Disable();
    txtLastresult->Disable();
    lstSteps->CreateColumns(frame, _("Step"), _("Comment"), 90);
    lstSteps->AddColumn(wxT("cmd"), 0);
    lstSteps->AddColumn(wxT("id"), 0);

    lstSchedules->CreateColumns(frame, _("Schedule"), _("Comment"), 90);
    lstSchedules->AddColumn(wxT("cmd"), 0);
    lstSchedules->AddColumn(wxT("id"), 0);
    btnChangeStep->Disable();
    btnRemoveStep->Disable();
    btnChangeSchedule->Disable();
    btnRemoveSchedule->Disable();
}


pgObject *dlgJob::GetObject()
{
    return job;
}


int dlgJob::Go(bool modal)
{
    pgSet *jcl=connection->ExecuteSet(wxT("SELECT jclname FROM pgagent.pga_jobclass"));
    if (jcl)
    {
        while (!jcl->Eof())
        {
            cbJobclass->Append(jcl->GetVal(0));
            jcl->MoveNext();
        }
        delete jcl;
    }

    if (job)
    {
        // edit mode
		recId = job->GetRecId();
		txtID->SetValue(NumToStr(recId));
        cbJobclass->SetValue(job->GetJobclass());
        chkEnabled->SetValue(job->GetEnabled());
        txtCreated->SetValue(DateToStr(job->GetCreated()));
        txtChanged->SetValue(DateToStr(job->GetChanged()));
        txtNextrun->SetValue(DateToStr(job->GetNextrun()));
        txtLastrun->SetValue(DateToStr(job->GetLastrun()));
        txtLastresult->SetValue(job->GetLastresult());

        wxCookieType cookie;
        pgObject *data=0;

        wxTreeItemId item, stepsItem, schedulesItem;
		item=mainForm->GetBrowser()->GetFirstChild(job->GetId(), cookie);
		while (item)
		{
			data=(pgObject*)mainForm->GetBrowser()->GetItemData(item);
            if (data->GetType() == PGA_STEPS) 
				stepsItem = item;

			if (data->GetType() == PGA_SCHEDULES) 
				schedulesItem = item;

			item=mainForm->GetBrowser()->GetNextChild(job->GetId(), cookie);
		}

		if (stepsItem)
		{
            pgCollection *coll=(pgCollection*)data;
            // make sure all columns are appended
            coll->ShowTreeDetail(mainForm->GetBrowser());
            // this is the columns collection
            item=mainForm->GetBrowser()->GetFirstChild(stepsItem, cookie);

            // add columns
            while (item)
            {
				data=(pgObject*)mainForm->GetBrowser()->GetItemData(item);
				if (data->GetType() == PGA_STEP)
				{
					pgaStep *step=(pgaStep*)data;
					int pos = lstSteps->AppendItem(PGAICON_STEP, step->GetName(), step->GetComment());
					lstSteps->SetItem(pos, 3, NumToStr((long)step));
					previousSteps.Add(NumToStr((long)step));
				}
				item=mainForm->GetBrowser()->GetNextChild(job->GetId(), cookie);
			}
		}

		if (schedulesItem)
		{
            pgCollection *coll=(pgCollection*)data;
            // make sure all columns are appended
            coll->ShowTreeDetail(mainForm->GetBrowser());
            // this is the columns collection
            item=mainForm->GetBrowser()->GetFirstChild(schedulesItem, cookie);

            // add columns
            while (item)
            {
				data=(pgObject*)mainForm->GetBrowser()->GetItemData(item);
				if (data->GetType() == PGA_SCHEDULE)
				{
					pgaSchedule *schedule=(pgaSchedule*)data;
					int pos = lstSchedules->AppendItem(PGAICON_SCHEDULE, schedule->GetName(), schedule->GetComment());
					lstSchedules->SetItem(pos, 3, NumToStr((long)schedule));
					previousSchedules.Add(NumToStr((long)schedule));
				}
				item=mainForm->GetBrowser()->GetNextChild(job->GetId(), cookie);
			}
		}


        CheckChange();
    }
    else
    {
        // create mode
        cbJobclass->SetSelection(0);
		btnChangeStep->Hide();
		btnChangeSchedule->Hide();
    }

    return dlgProperty::Go(modal);
}


pgObject *dlgJob::CreateObject(pgCollection *collection)
{
    pgObject *obj=pgaJob::ReadObjects(collection, 0, wxT("   WHERE jobid=") + NumToStr(recId) + wxT("\n"));
    return obj;
}


void dlgJob::CheckChange()
{
    bool enable=true;
    wxString name=GetName();
    if (job)
    {
        enable  =  txtComment->GetValue() != job->GetComment()
                || name != job->GetName()
                || chkEnabled->GetValue() != job->GetEnabled();
        if (!enable)
        {
            enable = !GetUpdateSql().IsEmpty();
        }
    }
    else
    {

    }
    CheckValid(enable, !name.IsEmpty(), _("Please specify name."));

    EnableOK(enable);
}


void dlgJob::OnChangeStep(wxCommandEvent &ev)
{
    long pos=lstSteps->GetSelection();
    pgaStep *obj=(pgaStep*) StrToLong(lstSteps->GetText(pos, 3));

    dlgStep step(mainForm, obj, job);
    step.CenterOnParent();
    step.SetConnection(connection);

    if (step.Go(true) >= 0)
    {
        lstSteps->SetItem(pos, 0, step.GetName());
        lstSteps->SetItem(pos, 1, step.GetComment());

        if (lstSteps->GetText(pos, 3).IsEmpty())
            lstSteps->SetItem(pos, 2, step.GetInsertSql());
        else
            lstSteps->SetItem(pos, 2, step.GetUpdateSql());

        CheckChange();
    }
}


void dlgJob::OnSelChangeStep(wxListEvent &ev)
{
    btnChangeStep->Enable();
    btnRemoveStep->Enable();
}


void dlgJob::OnAddStep(wxCommandEvent &ev)
{
    dlgStep step(mainForm, NULL, job);
    step.CenterOnParent();
    step.SetConnection(connection);
    if (step.Go(true) >= 0)
    {
        int pos = lstSteps->AppendItem(PGAICON_STEP, step.GetName(), step.GetComment());
        lstSteps->SetItem(pos, 2, step.GetInsertSql());
        CheckChange();
    }
}


void dlgJob::OnRemoveStep(wxCommandEvent &ev)
{
    lstSteps->DeleteCurrentItem();

    btnChangeStep->Disable();
    btnRemoveStep->Disable();

    CheckChange();
}


void dlgJob::OnSelChangeSchedule(wxListEvent &ev)
{
    btnChangeSchedule->Enable();
    btnRemoveSchedule->Enable();
}


void dlgJob::OnChangeSchedule(wxCommandEvent &ev)
{
    long pos=lstSchedules->GetSelection();
    pgaSchedule *obj=(pgaSchedule*) StrToLong(lstSchedules->GetText(pos, 3));

    dlgSchedule schedule(mainForm, obj, job);
    schedule.CenterOnParent();
    schedule.SetConnection(connection);

    if (schedule.Go(true) >= 0)
    {
        lstSchedules->SetItem(pos, 0, schedule.GetName());
        lstSchedules->SetItem(pos, 1, schedule.GetComment());

        if (lstSchedules->GetText(pos, 3).IsEmpty())
            lstSchedules->SetItem(pos, 2, schedule.GetInsertSql());
        else
            lstSchedules->SetItem(pos, 2, schedule.GetUpdateSql());

        CheckChange();
    }
}


void dlgJob::OnAddSchedule(wxCommandEvent &ev)
{
    dlgSchedule schedule(mainForm, NULL, job);
    schedule.CenterOnParent();
    schedule.SetConnection(connection);
    if (schedule.Go(true) >= 0)
    {
        int pos = lstSchedules->AppendItem(PGAICON_SCHEDULE, schedule.GetName(), schedule.GetComment());
        lstSchedules->SetItem(pos, 2, schedule.GetInsertSql());
        CheckChange();
    }
}


void dlgJob::OnRemoveSchedule(wxCommandEvent &ev)
{
    lstSchedules->DeleteCurrentItem();

    btnChangeSchedule->Disable();
    btnRemoveSchedule->Disable();

    CheckChange();
}


wxString dlgJob::GetInsertSql()
{
    wxString sql;

    if (!job)
    {
        sql = wxT("INSERT INTO pgagent.pga_job (jobid, jobjclid, jobname, jobdesc, jobenabled)\n")
              wxT("SELECT <id>, jcl.jclid, ") + qtString(GetName()) + 
              wxT(", ") + qtString(txtComment->GetValue()) + wxT(", ") + BoolToStr(chkEnabled->GetValue()) + wxT("\n")
              wxT("  FROM pgagent.pga_jobclass jcl WHERE jclname=") + qtString(cbJobclass->GetValue());
    }
    return sql;
}


wxString dlgJob::GetUpdateSql()
{
    wxString sql, name;
    name=GetName();

    if (job)
    {
        // edit mode
        wxString vars;

        if (name != job->GetName())
        {
            if (!vars.IsEmpty())
                vars.Append(wxT(", "));
            vars.Append(wxT("jobname = ") + qtString(name));
        }
        if (cbJobclass->GetValue().Trim() != job->GetJobclass())
        {
            if (!vars.IsEmpty())
                vars.Append(wxT(", "));
            vars.Append(wxT("jobjclid= (SELECT jclid FROM pgagent.pga_jobclass WHERE jclname=") + qtString(cbJobclass->GetValue()) + wxT(")"));
        }
        if (chkEnabled->GetValue() != job->GetEnabled())
        {
            if (!vars.IsEmpty())
                vars.Append(wxT(", "));
            vars.Append(wxT("jobenabled = ") + BoolToStr(chkEnabled->GetValue()));
        }
        if (txtComment->GetValue() != job->GetComment())
        {
            if (!vars.IsEmpty())
                vars.Append(wxT(", "));
            vars.Append(wxT("jobdesc = ") + qtString(txtComment->GetValue()));
        }

        if (!vars.IsEmpty())
            sql = wxT("UPDATE pgagent.pga_job SET ") + vars + wxT("\n")
                  wxT(" WHERE jobid=") + NumToStr(recId);

    }
    else
    {
        // create mode
        // done by GetInsertSql
    }

    int pos, index;

    wxArrayString tmpSteps = previousSteps;
    for (pos=0 ; pos < lstSteps->GetItemCount() ; pos++)
    {
        wxString str=lstSteps->GetText(pos, 3);
        if (!str.IsEmpty())
        {
            index=tmpSteps.Index(str);
            if (index >= 0)
                tmpSteps.RemoveAt(index);
        }
        str=lstSteps->GetText(pos, 2);
        if (!str.IsEmpty())
            sql += str + wxT(";\n");
    }

    for (index = 0 ; index < (int)tmpSteps.GetCount() ; index++)
    {
        sql += wxT("DELETE FROM pgagent.pga_jobstep WHERE jstid=") 
            + NumToStr(((pgaStep*)StrToLong(tmpSteps.Item(index)))->GetRecId()) + wxT(";\n");
    }

    wxArrayString tmpSchedules = previousSchedules;
    for (pos=0 ; pos < lstSchedules->GetItemCount() ; pos++)
    {
        wxString str=lstSchedules->GetText(pos, 3);
        if (!str.IsEmpty())
        {
            index=tmpSchedules.Index(str);
            if (index >= 0)
                tmpSchedules.RemoveAt(index);
        }
        str=lstSchedules->GetText(pos, 2);
        if (!str.IsEmpty())
            sql += str + wxT(";\n");
    }

    for (index = 0 ; index < (int)tmpSchedules.GetCount() ; index++)
    {
        sql += wxT("DELETE FROM pgagent.pga_schedule WHERE jscid=") 
            + NumToStr(((pgaStep*)StrToLong(tmpSchedules.Item(index)))->GetRecId()) + wxT(";\n");
    }

	return sql;

}


