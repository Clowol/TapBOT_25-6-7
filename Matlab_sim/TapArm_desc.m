%% ============================================================
% TapROT Arm Description  —— create robot(MDH)
% Structure：
%       Platform(2-DOF) + Steel Belt (1-DOF) + End-point platform(3-DOF)
%          ( R + R )    +           P        +       ( R + R + R )
%% MDH Parameter Table - [alpha, A, D, theta]
%  alpha:     连杆扭角 (rad)    |  A    :     连杆长度 (m)
%  D    :     连杆偏距 (m)      |  theta:     关节角初始值 (rad)  
%mdh_params  = [
%    0,      0,  0.0665,     0;
%    pi/2,   0,  0.195 ,     0;
%      0,    0,      0,      
%      0,    0,      0,      0,
%];
%
%   i | α_{i-1}  | a_{i-1}  | θ_i       | d_i      | 类型
%  ---|----------|----------|-----------|----------|------
%   1 |    0     |    0     | θ₁*       | L0       | R          云台Pan
%   2 |  -π/2    |    0     | θ₂*       | 0        | R          云台Tilt
%   3 |   π/2    |    0     | 0         | d₃*      | P          钢带伸缩
%   4 |    0     |    0     | θ₄*       | d_plat   | R          连杆1 (绕伸缩方向)
%   5 |  -π/2    |   L4     | θ₅*       | 0        | R          连杆2 (肘部)
%   6 |    0     |   L5     | θ₆*       | L6       | R          连杆3 (末端)
%
%% ===========  a. create  robot arm    ================
function robot = TapArm_desc()

fprintf('============================================\n');
fprintf('tap_spring_robot_arm MDH \n');
fprintf('关节: R-R-P-R-R-R (6-DOF)\n');
fprintf('============================================\n\n');

u = symunit;
L0 = 0.2615 * u.m;      % 底座高度
L_Min = 0.4 * u.m;
L_Max = 2.1 * u.m;
d_plat = 0.05 * u.m;
L_4 = 0.1 * u.m;
L_5 = 0.3 * u.m；
L_6 = 0.25 * u.m;

% Link([theta, d, a, alpha, sigma], 'modified') 
% -> sigma=0 旋转副, sigma=1 移动副
L1 = Link([0,     L0,    0,       0,    0],'modified');
L2 = Link([0,      0,    0,   -pi/2,    0],'modified');
L3 = Link([0,      0,    0,    pi/2,    1],'modified');
L4 = Link([0, d_plat,    0,       0,    0],'modified');
L5 = Link([0,      0,   L4,   -pi/2,    0],'modified');
L6 = Link([0,     L6,   L5,       0,    0],'modified');

% 设置仿真软限位
L1.qlim = [(240/180)*(-pi) , (120/180)*pi];
L2.qlim = [pi/4 , 3*pi/4];
L3.qlim = [ L_Min , L_Max ];        
L4.qlim = [ -pi , pi ];    % rotate continuously
L5.qlim = [ -pi/2 , pi/2 ];
L6.qlim = [ -pi/6 , pi/6 ];

robot = SerialLink([L1,L2], 'name', 'Tape Spring Robot');


% 显示模型信息
fprintf('============================================\n');
fprintf('机械臂模型已创建: %s\n', robot.name);
fprintf('关节数量: %d\n', robot.n);
fprintf('关节配置: %s\n', robot.config);
fprintf('============================================\n');

% 显示连杆参数表
fprintf('--- MDH 参数表 ---\n');
robot.display();

end
