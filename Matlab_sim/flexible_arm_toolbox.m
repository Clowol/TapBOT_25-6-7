%% ===== 1. 几何参数 =====
L0     = 0.15;   % 底座高度
L4     = 0.10;   % 连杆1长度 (绕伸缩方向旋转的那根)
L5     = 0.30;   % 连杆2长度 (肘部)
L6     = 0.25;   % 连杆3长度 (末端)
d_plat = 0.05;   % 伸缩末端平台偏距

%% ===== 2. 定义 MDH 连杆 =====
L(1) = Link([0,  L0,     0,  0],     'modified');         % Pan
L(2) = Link([0,  0,     0, -pi/2],  'modified');         % Tilt
L(3) = Link([0,  0,   0,  pi/2, 1], 'modified');       % 伸缩 (P)
L(4) = Link([0,  d_plat, 0,  0],     'modified');         % 连杆1: 绕伸缩方向
L(5) = Link([0,  0,      L4, -pi/2], 'modified');         % 连杆2: 肘部
L(6) = Link([0,  L6,     L5, 0],     'modified');         % 连杆3: 末端

% 关节限制
L(1).qlim = [-pi,   pi];
L(2).qlim = [pi/3, 2*pi/3];
L(3).qlim = [0.40,  2];
L(4).qlim = [-pi,   pi];       % 连杆1可连续旋转
L(5).qlim = [-pi/2, pi/2];
L(6).qlim = [-pi/2, pi/2];

robot = SerialLink(L, 'name', '柔性机械臂 (6-DOF)', ...
                   'comment', 'Pan-Tilt-伸缩-连杆1(绕伸缩)-连杆2(肘)-连杆3(末端)');

fprintf('--- MDH 参数表 ---\n');
robot.display();

% %% ===== 3. 正向运动学 =====
% fprintf('\n========== 正向运动学 ==========\n');
% 
% q_test = [0, pi/6, 0.50, pi/3, -pi/4, pi/6];
% T_end  = robot.fkine(q_test);
% 
% fprintf('关节角: [%.1f°, %.1f°, %.2fm, %.1f°, %.1f°, %.1f°]\n', rad2deg(q_test));
% 
% if isa(T_end, 'SE3')
%     fprintf('末端位置: [%.4f, %.4f, %.4f] m\n', T_end.t);
%     disp('末端位姿 T:');
%     disp(T_end.T);
% else
%     fprintf('末端位置: [%.4f, %.4f, %.4f] m\n', transl(T_end));
%     disp('末端位姿 T:');
%     disp(T_end);
% end
% 
% %% ===== 4. 雅可比 =====
% fprintf('\n========== 雅可比 (6×6) ==========\n');
% J0 = robot.jacob0(q_test);
% fprintf('线速度 Jacobian (前3行):\n');  disp(J0(1:3,:));
% fprintf('角速度 Jacobian (后3行):\n');  disp(J0(4:6,:));
% 
% %% ===== 5. 可视化 =====
% fprintf('\n========== 三维构型 ==========\n');
% figure('Name', '机械臂构型', 'NumberTitle', 'off', ...
%        'Position', [50, 80, 900, 700]);
% robot.plot(q_test, 'workspace', [-1.5 1.5 -1.5 1.5 0 2], ...
%            'scale', 0.5);
% title(sprintf('构型: q=[%.0f°,%.0f°,%.2fm,%.0f°,%.0f°,%.0f°]', rad2deg(q_test)));
% xlabel('X'); ylabel('Y'); zlabel('Z');
% 
% %% ===== 6. 逆运动学 (6-DOF, 可直接用 ikine) =====
% fprintf('\n========== 逆运动学 ==========\n');
% 
% q_target = [pi/4, pi/4, 0.5, pi/3, -pi/3, pi/6];
% T_target = robot.fkine(q_target);
% fprintf('目标: [%.1f°,%.1f°,%.2fm,%.1f°,%.1f°,%.1f°]\n', rad2deg(q_target));
% 
% q_init = [0, 0, 0.4, 0, 0, 0];
% 
% try
%     % 6-DOF 机器人不需要特殊 mask
%     q_ik = robot.ikine(T_target, 'q0', q_init, ...
%                         'ilimit', 500, 'tol', 1e-8);
%     fprintf('ikine:  [%.1f°,%.1f°,%.2fm,%.1f°,%.1f°,%.1f°]\n', rad2deg(q_ik));
% catch ME
%     fprintf('ikine 失败 (%s), 改用 ikcon...\n', ME.message);
%     q_ik = robot.ikcon(T_target, q_init);
%     fprintf('ikcon:  [%.1f°,%.1f°,%.2fm,%.1f°,%.1f°,%.1f°]\n', rad2deg(q_ik));
% end
% 
% T_verify = robot.fkine(q_ik);
% if isa(T_verify, 'SE3')
%     err_pos = norm(T_target.t - T_verify.t);
% else
%     err_pos = norm(transl(T_target) - transl(T_verify));
% end
% fprintf('位置误差: %.4e m\n', err_pos);
% 
% %% ===== 7. 工作空间 =====
% fprintf('\n========== 工作空间 ==========\n');
% N_ws = 5000;
% pts  = zeros(N_ws, 3);
% lims = [-pi,pi; -pi/3,pi/3; 0.20,0.80; -pi,pi; -pi/2,pi/2; -pi/2,pi/2];
% 
% fprintf('采样中...');
% for k = 1:N_ws
%     qk = lims(:,1)' + rand(1,6) .* (lims(:,2)' - lims(:,1)');
%     Tk = robot.fkine(qk);
%     pts(k,:) = double(transl(Tk));
% end
% fprintf(' 完成\n');
% 
% figure('Name', '工作空间', 'NumberTitle', 'off', ...
%        'Position', [100, 80, 900, 700]);
% scatter3(pts(:,1), pts(:,2), pts(:,3), 4, pts(:,3), ...
%          'filled', 'MarkerFaceAlpha', 0.12);
% colormap(jet); colorbar;
% xlabel('X'); ylabel('Y'); zlabel('Z');
% title(sprintf('工作空间 (N=%d)', N_ws));
% axis equal; grid on; view(30,30);
% 
% %% ===== 8. 轨迹规划 =====
% fprintf('\n========== 轨迹规划 ==========\n');
% 
% q_start = [0,   0,    0.25, 0,     0,     0];
% q_end   = [pi/3, pi/8, 0.60, pi/2, -pi/4, pi/6];
% N_pts   = 100;
% T_dur   = 3;
% 
% fprintf('起点: [%.1f°,%.1f°,%.2fm,%.1f°,%.1f°,%.1f°]\n', rad2deg(q_start));
% fprintf('终点: [%.1f°,%.1f°,%.2fm,%.1f°,%.1f°,%.1f°]\n', rad2deg(q_end));
% 
% [q_traj, qd_traj, qdd_traj] = jtraj(q_start, q_end, N_pts);
% 
% % 关节轨迹图
% figure('Name', '关节轨迹', 'NumberTitle', 'off', ...
%        'Position', [100, 100, 900, 700]);
% t_vec = linspace(0, T_dur, N_pts);
% jnames = {'Pan (°)','Tilt (°)','伸缩 (m)','连杆1 (°)','连杆2 (°)','连杆3 (°)'};
% q_deg = [rad2deg(q_traj(:,[1,2,4,5,6]))];
% q_plot = [q_deg(:,1:2), q_traj(:,3), q_deg(:,3:5)];
% 
% for j = 1:6
%     subplot(6,1,j);
%     plot(t_vec, q_plot(:,j), 'b-', 'LineWidth', 1.2);
%     ylabel(jnames{j});
%     if j == 1, title('jtraj 7次多项式轨迹'); end
%     if j == 6, xlabel('时间 (s)'); end
%     grid on;
% end
% 
% %% ===== 9. 轨迹动画 =====
% fprintf('\n========== 轨迹动画 ==========\n');
% figure('Name', '轨迹动画', 'NumberTitle', 'off', ...
%        'Position', [150, 80, 1000, 750]);
% 
% fps = 25;
% dt  = 1 / fps;
% 
% for k = 1:N_pts
%     robot.plot(q_traj(k,:), 'workspace', [-1.5 1.5 -1.5 1.5 0 2], ...
%                'scale', 0.5, 'delay', 0, 'nowrist');
%     title(sprintf('t = %.2f s / %.1f s', t_vec(k), T_dur));
%     drawnow;
%     pause(dt);
% end
% 
% %% ===== 10. 动力学 =====
% fprintf('\n========== 动力学 (示例参数) ==========\n');
% 
% robot.links(1).m = 1.5;   robot.links(1).r = [0; 0; L0/2];
% robot.links(2).m = 0.8;   robot.links(2).r = [0; 0; L1/2];
% robot.links(3).m = 0.3;   robot.links(3).r = [0; 0; 0.2];
% robot.links(4).m = 0.4;   robot.links(4).r = [0; 0; d_plat/2];
% robot.links(5).m = 1.0;   robot.links(5).r = [L4/2; 0; 0];
% robot.links(6).m = 0.6;   robot.links(6).r = [L5/2; 0; L6/2];
% 
% robot.gravity = [0; 0; -9.81];
% 
% q_dyn = q_test;
% tau   = robot.rne(q_dyn, zeros(1,6), zeros(1,6));
% fprintf('重力补偿力矩 (Nm):\n');
% fprintf('  [%.3f, %.3f, %.3f, %.3f, %.3f, %.3f]\n', tau);

%% ===== 11. 交互式示教 (放最后) =====
fprintf('\n========================================\n');
fprintf('自动示例完成。即将打开 teach 窗口。\n');
fprintf('关闭 teach 窗口后脚本结束。\n');
fprintf('========================================\n');

figure('Name', '交互式示教 (关闭此窗口结束)', 'NumberTitle', 'off', ...
       'Position', [100, 80, 900, 700]);
robot.teach(q_test);
fprintf('脚本结束。\n');
